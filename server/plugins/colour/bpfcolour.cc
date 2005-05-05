#include <stdio.h>
#include <map>
#include <list>
#include <string>
#include <vector>
#include "bpfcolour.h"
#include <libtrace.h>
#include "colours.h"
extern FILE *colour_in;
extern "C"
int colour_parse();

char next_id=0;

struct info_t {
	std::string name;
	std::string exp;
	struct colour_t colour;
	struct libtrace_filter_t *filter;
	char id;
};

std::map<char, std::pair< std::string, struct colour_t> > id2name_colour;
std::map<std::pair<std::string,struct colour_t>,char> name_colour2id;
typedef std::vector<struct info_t> exps_t;
exps_t exps;

bool operator<(
		const std::pair<std::string,struct colour_t> &a,
		const std::pair<std::string,struct colour_t> &b)
{
	if (a.first != b.first)		   return a.first < b.first;
	if (a.second.red != b.second.red)  return a.second.red < b.second.red;
	if (a.second.red != b.second.blue) return a.second.blue < b.second.blue;
	return a.second.green < b.second.green;
}

void add_expression(const char *name, struct colour_t colour, const char *exp)
{
	std::pair<std::string,struct colour_t> id(name,colour);
	if (name_colour2id.find(id) == name_colour2id.end()) {
		name_colour2id[id]=next_id++;
		id2name_colour[name_colour2id[id]]=id;
	}
	struct info_t info;
	info.colour = colour;
	info.name = name;
	info.exp = exp;
	info.filter = trace_bpf_setfilter(exp);
	if (!info.filter) {
		fprintf(stderr,"BPF Filter doesn't parse: %s\n",info.exp.c_str());
		return;
	}
	info.id = name_colour2id[id];
	exps.push_back(info);
}

extern "C"
int init_module(char *args) 
{
	colour_in = fopen(args,"r");
	printf("Reading bpfcolour config from `%s'\n",args);
	if (!colour_in) { 
		fprintf(stderr,"Unable to parse config file %s\n",args);
		return 0; 
	}
	colour_parse();
	fclose(colour_in);
	return 1;
}

extern "C"
int mod_get_colour(unsigned char *id_num, struct libtrace_packet_t *packet)
{
	for(exps_t::const_iterator i=exps.begin();
			i!=exps.end();
			++i) {
		if (trace_bpf_filter((i)->filter,packet)) {
			*id_num=(i)->id;
			return 0;
		}
	}
	return 1;
}

extern "C"
void mod_get_info(uint8_t colours[3], char name[256], int id)
{
	if (id >= next_id) {
		colours[0] = colours[1] = colours[2] = 0;
		strcpy(name,"NULL");
		return;
	}

	colours[0] = id2name_colour[id].second.red;
	colours[1] = id2name_colour[id].second.blue;
	colours[2] = id2name_colour[id].second.green;
	strncpy(name,id2name_colour[id].first.c_str(),256);
	name[256-1]='\0';
	return;
}

int main(int argc, char *argv[])
{
	init_module(argv[1]);
}
