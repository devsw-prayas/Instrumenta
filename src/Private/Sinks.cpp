#include "Sinks.h"

#include <iostream>
#include <mutex>

#include "LogUtils.h"

namespace instrumenta {
	// ConsoleSink Implementation
	ConsoleSink::ConsoleSink(bool colors) : enabled(true), minLevel(E_LogLevel::DEBUG_), useColors(colors) {}

	void ConsoleSink::write(const LogEntry& entry) {
		if (!enabled || entry.level < minLevel) return;

		std::lock_guard<std::mutex> lock(consoleMutex);
		if (useColors) {
			std::cout << entry.toColoredString() << "\n";
		}
		else {
			std::cout << entry.toString() << "\n";
		}
	}

	void ConsoleSink::flush() {
		std::lock_guard<std::mutex> lock(consoleMutex);
		std::cout.flush();
	}

	bool ConsoleSink::isEnabled() const {
		return enabled;
	}

	void ConsoleSink::setEnabled(bool enable) {
		enabled = enable;
	}

	E_LogLevel ConsoleSink::getMinLevel() const {
		return minLevel;
	}

	void ConsoleSink::setMinLevel(E_LogLevel level) {
		minLevel = level;
	}

	void ConsoleSink::setUseColors(bool colors) {
		useColors = colors;
	}

	// FileSink Implementation
	FileSink::FileSink(const std::string& filename, bool autoFlush)
		: filename(filename), enabled(true), minLevel(E_LogLevel::DEBUG_), autoFlush(autoFlush) {
		fileStream.open(filename, std::ios::out | std::ios::app);
		if (!fileStream.is_open()) {
			throw std::runtime_error("Failed to open log file: " + filename);
		}
	}

	FileSink::~FileSink() {
		if (fileStream.is_open()) {
			std::lock_guard<std::mutex> lock(fileMutex);
			if (!fileStream.is_open()) return;

			for (const auto& entry : buffer) {
				fileStream << entry.toString() << "\n";
			}
			buffer.clear();
			fileStream.flush();
			fileStream.close();
		}
	}

	void FileSink::write(const LogEntry& entry) {
		if (!enabled || entry.level < minLevel || !fileStream.is_open()) return;

		std::lock_guard<std::mutex> lock(fileMutex);
		if (autoFlush.load()) {
			fileStream << entry.toString() << "\n";
			fileStream.flush();
		}
		else {
			buffer.push_back(entry);
		}
	}

	void FileSink::flush() {
		std::lock_guard<std::mutex> lock(fileMutex);
		if (!fileStream.is_open()) return;

		for (const auto& entry : buffer) {
			fileStream << entry.toString() << "\n";
		}
		buffer.clear();
		fileStream.flush();
	}

	bool FileSink::isEnabled() const {
		return enabled;
	}

	void FileSink::setEnabled(bool enable) {
		enabled = enable;
	}

	E_LogLevel FileSink::getMinLevel() const {
		return minLevel;
	}

	void FileSink::setMinLevel(E_LogLevel level) {
		minLevel = level;
	}

	void FileSink::setAutoFlush(bool enable) {
		autoFlush.store(enable);
	}

	bool FileSink::isOpen() const {
		return fileStream.is_open();
	}

	// BufferedSink Implementation
	BufferedSink::BufferedSink(std::unique_ptr<ILogSink> sink, size_t bufferSize,
		std::chrono::milliseconds flushInterval)
		: underlyingSink(std::move(sink)), bufferSize(bufferSize), shouldStop(false),
		flushInterval(flushInterval) {
		buffer.reserve(bufferSize);
		flushThread = std::thread(&BufferedSink::flushWorker, this);
	}

	BufferedSink::~BufferedSink() {
		shouldStop.store(true);
		flushCV.notify_all();
		if (flushThread.joinable()) {
			flushThread.join();
		}
		std::unique_lock<std::mutex> lock(bufferMutex);
		if (!underlyingSink || buffer.empty()) return;

		for (const auto& entry : buffer) {
			underlyingSink->write(entry);
		}
		buffer.clear();
		underlyingSink->flush();
	}

	void BufferedSink::write(const LogEntry& entry) {
		if (!underlyingSink || !underlyingSink->isEnabled() ||
			entry.level < underlyingSink->getMinLevel()) return;

		std::lock_guard<std::mutex> lock(bufferMutex);
		buffer.push_back(entry);
		if (buffer.size() >= bufferSize) {
			flushCV.notify_one();
		}
	}

	void BufferedSink::flush() {
		std::unique_lock<std::mutex> lock(bufferMutex);
		if (!underlyingSink || buffer.empty()) return;

		for (const auto& entry : buffer) {
			underlyingSink->write(entry);
		}
		buffer.clear();
		underlyingSink->flush();
	}

	bool BufferedSink::isEnabled() const {
		return underlyingSink ? underlyingSink->isEnabled() : false;
	}

	void BufferedSink::setEnabled(bool enable) {
		if (underlyingSink) underlyingSink->setEnabled(enable);
	}

	E_LogLevel BufferedSink::getMinLevel() const {
		return underlyingSink ? underlyingSink->getMinLevel() : E_LogLevel::DEBUG_;
	}

	void BufferedSink::setMinLevel(E_LogLevel level) {
		if (underlyingSink) underlyingSink->setMinLevel(level);
	}

	void BufferedSink::flushWorker() {
		while (!shouldStop.load()) {
			std::unique_lock<std::mutex> lock(bufferMutex);
			flushCV.wait_for(lock, flushInterval, [this] {
				return shouldStop.load() || !buffer.empty();
				});

			if (!buffer.empty()) {
				for (const auto& entry : buffer) {
					if (underlyingSink) underlyingSink->write(entry);
				}
				buffer.clear();
				if (underlyingSink) underlyingSink->flush();
			}
		}
	}

	bool TagRouter::route(const LogEntry& entry, std::weak_ptr<std::unordered_map<std::string, std::unique_ptr<ILogSink>>>sinks) {
		auto sinksPtr = sinks.lock();
		if (sinksPtr->contains(tag)) {
			auto sink = sinksPtr->find(tag);
			sink->second->write(entry);
			return true;
		}
		return false;
	}

	std::unique_ptr<TagRouter> TagRouter::getInstance(const std::string& tag) {
		return std::make_unique<TagRouter>(tag);
	}
}