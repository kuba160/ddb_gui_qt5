#include <libintl.h>
#include <stdio.h>

int main() {
	bind_textdomain_codeset ("hello", "");
	printf(gettext("Hello World!"));
	return 0;
}
