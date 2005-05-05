#ifndef BPFCOLOUR_H
#define BPFCOLOUR_H
struct colour_t {
	char red;
	char green;
	char blue;
};

#ifdef __cplusplus
extern "C"
#endif
void add_expression(const char *name, struct colour_t colour, const char *exp);

#endif
