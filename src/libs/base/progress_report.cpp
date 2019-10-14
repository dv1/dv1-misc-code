#include <assert.h>
#include "fmt/ostream.h"
#include "progress_report.hpp"


namespace base
{


timed_progress_report::timed_progress_report(std::chrono::steady_clock::duration p_min_time_between_reports, progress_report_callback p_progress_report_callback)
	: m_min_time_between_reports(p_min_time_between_reports)
	, m_progress_report_callback(std::move(p_progress_report_callback))
{
	assert(m_progress_report_callback);
}


void timed_progress_report::operator()(std::size_t p_progress, std::size_t p_max_progress) const
{
	auto now = std::chrono::steady_clock::now();
	auto time_since_last_report = now - m_last_report_time_point;

	if ((time_since_last_report >= m_min_time_between_reports) || (p_progress == p_max_progress))
	{
		m_progress_report_callback(p_progress, p_max_progress);
		m_last_report_time_point = now;
	}
}


timed_progress_report make_ostream_progress_report(std::ostream &p_ostream, std::string p_text, std::chrono::steady_clock::duration p_min_time_between_reports)
{
	return timed_progress_report {
		p_min_time_between_reports,
		[text = std::move(p_text), &p_ostream](unsigned long p_progress, unsigned long p_max_progress) -> void {
			int progress_percent = p_progress * 100 / p_max_progress;
			fmt::print(p_ostream, "{}: {}%\r", text, progress_percent);
		}
	};
}


} // namespace base end
