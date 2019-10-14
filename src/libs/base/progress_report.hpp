#ifndef PROGRESS_REPORT_HPP_________
#define PROGRESS_REPORT_HPP_________

#include <chrono>
#include <iostream>
#include <functional>


namespace base
{


typedef std::function < void(unsigned long p_progress, unsigned long p_max_progress) > progress_report_callback;


class timed_progress_report
{
public:
	explicit timed_progress_report(std::chrono::steady_clock::duration p_min_time_between_reports, progress_report_callback p_progress_report_callback);
	void operator()(unsigned long p_progress, unsigned long p_max_progress) const;


private:
	std::chrono::steady_clock::duration m_min_time_between_reports;
	progress_report_callback m_progress_report_callback;

	mutable std::chrono::steady_clock::time_point m_last_report_time_point;
};


timed_progress_report make_ostream_progress_report(std::ostream &p_ostream, std::string p_text, std::chrono::steady_clock::duration p_min_time_between_reports);


} // namespace base end


#endif // PROGRESS_REPORT_HPP_________
