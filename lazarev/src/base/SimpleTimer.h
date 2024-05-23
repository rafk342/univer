#pragma once
#include <chrono>


class SimpleTimer
{
	using TClock = std::chrono::high_resolution_clock;
	using TTime = std::chrono::time_point<std::chrono::high_resolution_clock>;

	TTime m_StartTime;
	TTime m_EndTime;

	bool m_IsRunning = false;

public:

	SimpleTimer()
	{
		Reset();
	}

	static SimpleTimer StartNew()
	{
		SimpleTimer result;
		result.Start();
		return result;
	}

	void Start()
	{
		m_StartTime = TClock::now();
		m_IsRunning = true;
	}

	void Stop()
	{
		if (!m_IsRunning)
			return;

		m_EndTime = TClock::now();
		m_IsRunning = false;
	}

	void Restart()
	{
		Reset();
		Start();
	}

	void Reset()
	{
		TTime time = TClock::now();
		m_StartTime = time;
		m_EndTime = time;
	}

	bool GetIsRunning() const { return m_IsRunning; }

	long long GetElapsedTicks() const
	{
		return (m_EndTime - m_StartTime).count();
	}

	double GetElapsedMinutes() const
	{
		return std::chrono::duration_cast<std::chrono::duration<double, std::ratio<60>>>(m_EndTime - m_StartTime).count();
	}

	double GetElapsedSeconds() const
	{
		double duration = std::chrono::duration_cast<std::chrono::duration<double>>(m_EndTime - m_StartTime).count();
		return duration;
	}

	double GetElapsedMilliseconds() const
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(m_EndTime - m_StartTime).count();
	}

	double GetElapsedMicroseconds() const
	{
		return std::chrono::duration_cast<std::chrono::microseconds>(m_EndTime - m_StartTime).count();
	}
};
