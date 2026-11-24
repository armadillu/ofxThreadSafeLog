#include "ofxThreadSafeLog.h"
#include <cstdint>
#include <map>
#include <thread>

ofxThreadSafeLog::ofxThreadSafeLog(): slots(new Slot[capacity]) {
	for (std::size_t i = 0; i < capacity; ++i) slots[i].sequence.store(i, std::memory_order_relaxed);
	startThread();
}

ofxThreadSafeLog* ofxThreadSafeLog::one() {
	// Kept alive until process exit, as in the original implementation.
	static ofxThreadSafeLog* instance = new ofxThreadSafeLog();
	return instance;
}

void ofxThreadSafeLog::setPrintToConsole(bool print) {
	alsoPrintToConsole.store(print, std::memory_order_relaxed);
}

// Bounded MPMC ring using per-slot sequence numbers, following the design at
// https://www.1024cores.net/home/lock-free-algorithms/queues/bounded-mpmc-queue
// A stalled producer can
// delay the consumer; a full queue applies backpressure rather than dropping logs.
bool ofxThreadSafeLog::tryEnqueue(Entry &entry) {
	std::size_t pos = enqueuePos.load(std::memory_order_relaxed);
	for (;;) {
		Slot &slot = slots[pos % capacity];
		const std::size_t seq = slot.sequence.load(std::memory_order_acquire);
		const std::intptr_t diff = static_cast<std::intptr_t>(seq - pos);
		if (diff == 0) {
			if (enqueuePos.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
				slot.entry = std::move(entry);
				slot.sequence.store(pos + 1, std::memory_order_release);
				return true;
			}
		} else if (diff < 0) return false;
		else pos = enqueuePos.load(std::memory_order_relaxed);
	}
}

bool ofxThreadSafeLog::tryDequeue(Entry &entry) {
	std::size_t pos = dequeuePos.load(std::memory_order_relaxed);
	for (;;) {
		Slot &slot = slots[pos % capacity];
		const std::size_t seq = slot.sequence.load(std::memory_order_acquire);
		const std::intptr_t diff = static_cast<std::intptr_t>(seq - (pos + 1));
		if (diff == 0) {
			if (dequeuePos.compare_exchange_weak(pos, pos + 1, std::memory_order_relaxed)) {
				entry = std::move(slot.entry);
				slot.sequence.store(pos + capacity, std::memory_order_release);
				return true;
			}
		} else if (diff < 0) return false;
		else pos = dequeuePos.load(std::memory_order_relaxed);
	}
}

void ofxThreadSafeLog::append(std::string logFile, std::string line) {
	std::size_t state = producers.load(std::memory_order_relaxed);
	do {
		if (state & closedBit) return;
	} while (!producers.compare_exchange_weak(state, state + 1, std::memory_order_acquire, std::memory_order_relaxed));
	struct ProducerGuard {
		std::atomic<std::size_t> &count;
		~ProducerGuard() { count.fetch_sub(1, std::memory_order_release); }
	} guard{producers};
	if (alsoPrintToConsole.load(std::memory_order_relaxed)) {
		ofLogNotice(ofFilePath::getFileName(logFile)) << line;
	}
	Entry entry{std::move(logFile), std::move(line)};
	while (!tryEnqueue(entry)) std::this_thread::yield();
}

void ofxThreadSafeLog::close() {
	producers.fetch_or(closedBit, std::memory_order_acq_rel);
	// Keep the consumer running until all admitted producers have enqueued.
	while ((producers.load(std::memory_order_acquire) & ~closedBit) != 0) std::this_thread::yield();
	stopThread();
	waitForThread();
}

void ofxThreadSafeLog::threadedFunction() {
#ifdef TARGET_WIN32
#elif defined(TARGET_LINUX)
	pthread_setname_np(pthread_self(), "ofxThreadSafeLog");
#else
	pthread_setname_np("ofxThreadSafeLog");
#endif
	std::map<std::string, std::unique_ptr<ofFile> > logs;
	Entry entry;
	for (;;) {
		if (tryDequeue(entry)) {
			auto it = logs.find(entry.file);
			if (it == logs.end()) {
				std::unique_ptr<ofFile> file(new ofFile());
				file->open(entry.file, ofFile::WriteOnly, false);
				it = logs.emplace(std::move(entry.file), std::move(file)).first;
			}
			*it->second << ofGetTimestampString("%Y/%m/%d %H:%M:%S") << " - " << entry.line << std::endl;
		} else if (!isThreadRunning()) {
			break;
		} else {
			ofSleepMillis(16);
		}
	}
}
