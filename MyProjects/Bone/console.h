#ifndef CONSOLE_H
#define CONSOLE_H

class consoleclass {

	public:
		HWND consolewindow;
		char *printbuf;

		consoleclass();
		~consoleclass();
		int handleconsole(HWND w_ptr, UINT uMsg, WPARAM wParam, LPARAM lParam);
		void createwindow(HWND w_ptr);
		void startup();
		void resize(LONG y);
		void clearconsole();
		void print(char *string);
		void printc(char *string);
	private:
		void scrolldown();
		};

#endif /* CONSOLE_H */
