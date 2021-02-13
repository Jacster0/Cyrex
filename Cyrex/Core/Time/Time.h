#pragma once
#include <chrono>  
#include <ctime>   
#include <sstream> 
#include <iomanip> 
#include <string> 

namespace Cyrex::crxtime {
	inline std::string GetCurrentTimeAsFormatedString() {
		auto now = std::chrono::system_clock::now();
		auto in_time_t = std::chrono::system_clock::to_time_t(now);

		std::stringstream ss;
		ss << std::put_time(std::localtime(&in_time_t), "%X");
		return ss.str();
	}

	inline std::string GetCurrentDateAsFormatedString() {
		auto now = std::chrono::system_clock::now();
		auto in_time_t = std::chrono::system_clock::to_time_t(now);

		std::stringstream ss;
		ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d");
		return ss.str();
	}

	inline std::string GetCurrentDateTimeAsFormatedString() {
		auto now = std::chrono::system_clock::now();
		auto in_time_t = std::chrono::system_clock::to_time_t(now);

		std::stringstream ss;
		ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
		return ss.str();
	}

	inline std::wstring GetCurrentTimeAsWideFormatedString() {
		auto now = std::chrono::system_clock::now();
		auto in_time_t = std::chrono::system_clock::to_time_t(now);

		std::wstringstream ss;
		ss << std::put_time(std::localtime(&in_time_t), L"%X");
		return ss.str();
	}

	inline std::wstring GetCurrentDateAsWideFormatedString() {
		auto now = std::chrono::system_clock::now();
		auto in_time_t = std::chrono::system_clock::to_time_t(now);

		std::wstringstream ss;
		ss << std::put_time(std::localtime(&in_time_t), L"%Y-%m-%d");
		return ss.str();
	}

	inline std::wstring GetCurrentDateTimeAsWideFormatedString() {
		auto now = std::chrono::system_clock::now();
		auto in_time_t = std::chrono::system_clock::to_time_t(now);

		std::wstringstream ss;
		ss << std::put_time(std::localtime(&in_time_t), L"%Y-%m-%d %X");
		return ss.str();
	}
}
