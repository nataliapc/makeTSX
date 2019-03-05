

#if defined(__linux__) || defined(__APPLE__) && defined(__MACH__)

	#define TXT_RESET		"\e[0m"
	#define TXT_BLINK		"\e[5m"

	#define TXT_BLACK		"\033[0;30m"
	#define TXT_RED			"\033[0;31m"
	#define TXT_GREEN		"\033[0;32m"
	#define TXT_YELLOW		"\033[0;33m"
	#define TXT_BLUE		"\033[0;34m"
	#define TXT_PURPLE		"\033[0;35m"
	#define TXT_CYAN		"\033[0;36m"
	#define TXT_WHITE		"\033[0;37m"

	#define TXT_B_BLACK		"\033[30;1m"
	#define TXT_B_RED		"\033[31;1m"
	#define TXT_B_GREEN		"\033[32;1m"
	#define TXT_B_YELLOW	"\033[33;1m"
	#define TXT_B_BLUE		"\033[34;1m"
	#define TXT_B_PURPLE	"\033[35;1m"
	#define TXT_B_CYAN		"\033[36;1m"
	#define TXT_B_WHITE		"\033[37;1m"

	#define BG_BLACK		"\033[0;40m"
	#define BG_RED			"\033[0;41m"
	#define BG_GREEN		"\033[0;42m"
	#define BG_YELLOW		"\033[0;43m"
	#define BG_BLUE			"\033[0;44m"
	#define BG_PURPLE		"\033[0;45m"
	#define BG_CYAN			"\033[0;46m"
	#define BG_WHITE		"\033[0;47m"

#elif defined(_WIN32) || defined(_WIN64)

	#define TXT_RESET		""
	#define TXT_BLINK		""

	#define TXT_BLACK		""
	#define TXT_RED			""
	#define TXT_GREEN		""
	#define TXT_YELLOW		""
	#define TXT_BLUE		""
	#define TXT_PURPLE		""
	#define TXT_CYAN		""
	#define TXT_WHITE		""

	#define TXT_B_BLACK		""
	#define TXT_B_RED		""
	#define TXT_B_GREEN		""
	#define TXT_B_YELLOW	""
	#define TXT_B_BLUE		""
	#define TXT_B_PURPLE	""
	#define TXT_B_CYAN		""
	#define TXT_B_WHITE		""

	#define BG_BLACK		""
	#define BG_RED			""
	#define BG_GREEN		""
	#define BG_YELLOW		""
	#define BG_BLUE			""
	#define BG_PURPLE		""
	#define BG_CYAN			""
	#define BG_WHITE		""

#else

	#define TXT_RESET		""
	#define TXT_BLINK		""

	#define TXT_BLACK		""
	#define TXT_RED			""
	#define TXT_GREEN		""
	#define TXT_YELLOW		""
	#define TXT_BLUE		""
	#define TXT_PURPLE		""
	#define TXT_CYAN		""
	#define TXT_WHITE		""

	#define TXT_B_BLACK		""
	#define TXT_B_RED		""
	#define TXT_B_GREEN		""
	#define TXT_B_YELLOW	""
	#define TXT_B_BLUE		""
	#define TXT_B_PURPLE	""
	#define TXT_B_CYAN		""
	#define TXT_B_WHITE		""

	#define BG_BLACK		""
	#define BG_RED			""
	#define BG_GREEN		""
	#define BG_YELLOW		""
	#define BG_BLUE			""
	#define BG_PURPLE		""
	#define BG_CYAN			""
	#define BG_WHITE		""

#endif

