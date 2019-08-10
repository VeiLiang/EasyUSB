/*
* main.c
*
*  Created on: 2018Äê10ÔÂ29ÈÕ
*  Author: VeiLiang
*/


int main(int argc, char **argv)
{
	usb_device_init(2);
	while(1);
}
void __fatal_error(const char *msg) {
    while (1);//board reset
}
#ifndef NDEBUG
void __assert_func(const char *file, int line, const char *func, const char *expr) {
    //printf("Assertion '%s' failed, at file %s:%d\n", expr, file, line);
    __fatal_error("Assertion failed");
}
#endif
