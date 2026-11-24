#ifndef __CollectionTable__ofxThreadSafeLog__
#define __CollectionTable__ofxThreadSafeLog__

#include "ofMain.h"
#include <atomic>
#include <cstddef>
#include <memory>
#include <string>
#include <utility>

class ofxThreadSafeLog: public ofThread {
public:
	static ofxThreadSafeLog* one();

	// Each file is truncated on its first use in a session. Rvalues are moved.
	void append(std::string logFile, std::string line);
	void close(); // Call after logging is finished; drains accepted messages.
	void setPrintToConsole(bool print);

private:
	struct Entry {
		std::string file;
		std::string line;
	};
	struct Slot {
		std::atomic<std::size_t> sequence;
		Entry entry;
	};
	static const std::size_t capacity = 4096;
	static const std::size_t closedBit = std::size_t(1) << (sizeof(std::size_t) * 8 - 1);

	ofxThreadSafeLog();
	bool tryEnqueue(Entry &entry);
	bool tryDequeue(Entry &entry);
	void threadedFunction();

	std::unique_ptr<Slot[]> slots;
	std::atomic<std::size_t> enqueuePos{0};
	std::atomic<std::size_t> dequeuePos{0};
	// High bit closes admission; remaining bits count producers already admitted.
	std::atomic<std::size_t> producers{0};
	std::atomic<bool> alsoPrintToConsole{false};
};
#endif
