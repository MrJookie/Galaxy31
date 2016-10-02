#ifndef CURSES_CONSOLE_HPP
#define CURSES_CONSOLE_HPP

#include <vector>
#include <string>
namespace curses {
	#include <ncurses.h>

struct ncurses_stream;
class CursesConsole {
	public:
	
		CursesConsole();
		void Refresh();
		void Print(std::string str);
		void Print(char ch);
		std::string Input();
		void StartCurses();
		void StopCurses();
		void SetCodeCompleteHandler( std::string (*handler)(std::string cmd, int cursor) );
		void SetInfoString(std::string str);
	private:
		std::string (*code_complete_handler)(std::string cmd, int cursor);
		int get_log_height();
		int get_info_height();
		friend struct ncurses_stream;
		curses::WINDOW* m_window;
		ncurses_stream* m_stream;
		std::vector<std::string> m_lines;
		
		// command and history
		std::string m_command;
		std::string m_last_command;
		int m_command_cursor;
		int m_history_counter;
		std::vector<std::string> m_history;
		
		std::string m_info_string;
		
		int m_window_height, m_window_width;
		int m_cursor; // only y dimension
		bool active;
		bool dirty;
};


#include "CursesConsole.hpp"
#include <iostream>

using namespace curses;

class ncursesbuf: public std::streambuf {
	public:
		void SetConsole(CursesConsole* cons) { this->cons = cons; }
		ncursesbuf() { cons = 0; }
		virtual int overflow(int c) {
			cons->Print((char)c);
		}
	private:
		CursesConsole* cons;
};

class ncurses_stream : public std::ostream {
	public:
		ncursesbuf tbuf;
		std::ostream &src;
		std::streambuf* const old_buf;

		ncurses_stream(CursesConsole* cons, std::ostream &o) : 
			src(o), 
			old_buf(o.rdbuf()),
			std::ostream(&tbuf) 
		{
			tbuf.SetConsole(cons);
			o.rdbuf(rdbuf());
		}

		~ncurses_stream() {
			src.rdbuf(old_buf);
		}
};

void CursesConsole::StartCurses() {
	initscr();
	cbreak();
	int h,w;
	w = COLS;
	h = LINES;
	m_window = newwin(h,w,0,0);
	keypad(m_window, true);
	if(!m_stream)
		m_stream = new ncurses_stream(this, std::cout);
	m_window_width = w;
	m_window_height = h;
	m_lines.emplace_back("");
	m_command = "/";
	m_command_cursor = 1;
	active = true;
}

void CursesConsole::StopCurses() {
	if(m_stream) {
		delete ((ncurses_stream*)m_stream);
		m_stream = 0;
	}
	endwin();
	active = false;
}

void CursesConsole::SetInfoString(std::string str) {
	m_info_string = str;
	dirty = true;
}

int CursesConsole::get_log_height() {
	return m_window_height-1-m_command.size()/m_window_width-1 - get_info_height();
}

int CursesConsole::get_info_height() {
	return m_info_string.size() / m_window_width + (!m_info_string.empty() ? 2 : 0);
}

void CursesConsole::Refresh() {
	if(dirty && m_window && active) {
		dirty = false;
		wclear(m_window);
		
		int curs = 0;
		int log_height = get_log_height();
		for(auto it = m_lines.begin()+m_cursor; curs < log_height && it != m_lines.end(); it++) {
			wmove(m_window, curs++, 0);
			wprintw(m_window, it->c_str());
		}
		
		int info_height = get_info_height();
		if(info_height > 0) {
			mvwhline(m_window, log_height, 0, ACS_HLINE, m_window_width);
			mvwprintw(m_window, log_height+1, 0, m_info_string.c_str());
		}
		
		mvwhline(m_window, log_height+info_height, 0, ACS_HLINE, m_window_width);
		mvwprintw(m_window, log_height+info_height+1, 0, m_command.c_str());
		wmove(m_window, log_height+info_height+1+m_command_cursor/m_window_width, m_command_cursor % m_window_width);
		wrefresh(m_window);
	}
}

void CursesConsole::Print(std::string str) {
	for(auto c : str) {
		Print(c);
	}
	dirty = true;
}

void CursesConsole::Print(char ch) {
	if(ch == '\n')
		m_lines.emplace_back("");
	else {
		if(m_lines.back().size() >= m_window_width)
			m_lines.emplace_back("");
		m_lines.back() += ch;
	}
	int log_height = get_log_height();
	if(m_lines.size() > log_height)
		m_cursor = m_lines.size() - log_height - 1;
	dirty = true;
}

CursesConsole::CursesConsole() {
	dirty = false;
	m_stream = 0;
	m_window = 0;
	m_cursor = 0;
	code_complete_handler = 0;
	m_history_counter = 0;
	active = false;
}

void CursesConsole::SetCodeCompleteHandler( std::string (*handler)(std::string cmd, int cursor) ) {
	code_complete_handler = handler;
}

std::string CursesConsole::Input() {
	while(1) {
		int input = wgetch(m_window);
		switch(input) {
			case KEY_UP:
				if(m_history_counter == 0) 
					break;
				if(m_history_counter == m_history.size()) {
					m_last_command = m_command;
				}
				m_command = m_history[--m_history_counter];
				m_command_cursor = m_command.size();
				break;
			case KEY_DOWN:
				if(m_history_counter >= m_history.size())
					break;
				if(m_history_counter == m_history.size()-1) {
					m_command = m_last_command;
					m_history_counter = m_history.size();
				} else {
					m_command = m_history[++m_history_counter];
				}
				m_command_cursor = m_command.size();
				break;
			case KEY_PPAGE:
				m_cursor = std::max<int>(m_cursor - get_log_height(), 0);
				break;
			case KEY_NPAGE:
				m_cursor = std::min<int>(m_cursor + get_log_height(), std::max<int>(0, m_lines.size() - get_log_height() - 1) );
				break;
			case KEY_EOL:
				m_cursor = m_lines.size() - get_log_height() - 1;
				break;
			case KEY_HOME:
				m_command_cursor = 1;
				break;
			case KEY_END:
				m_command_cursor = m_command.size();
				break;
			case KEY_BACKSPACE:
			case 127:
				if(m_command_cursor > 1) {
					m_command_cursor--;
					m_command.erase(m_command_cursor, 1);
				}
				break;
			case KEY_LEFT:
				if(m_command_cursor > 1)
					m_command_cursor--;
				break;
			case KEY_RIGHT:
				if(m_command_cursor < m_command.size())
					m_command_cursor++;
				break;
			case '\t':
				if(code_complete_handler) {
					std::string complete = code_complete_handler(m_command, m_command.size()-1);
					m_command.insert(m_command_cursor, complete);
					m_command_cursor += complete.size();
				}
				break;
			case KEY_DL:
			case 21:
				m_command = "/";
				m_command_cursor = 1;
				break;
			case KEY_MOUSE:
				Print("its mouse\n");
				break;
			case KEY_RESIZE:
				getmaxyx(stdscr, m_window_height, m_window_width);
				break;
			default:
				if(input == '\n') {
					std::string cmd = m_command.substr(1);
					
					m_history.push_back(m_command);
					m_history_counter = m_history.size();
					m_command_cursor = 1;
					
					m_command = "/";
					return cmd;
				} else {
					m_command.insert(m_command_cursor++, 1, input);
				}
		}
		dirty = true;
	}
}

}
using curses::CursesConsole;
#endif
