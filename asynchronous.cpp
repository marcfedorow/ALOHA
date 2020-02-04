#include <iostream>
#include <queue>
#include <vector>
#include <cstdlib>
#include <ctime>
using namespace std;
#define random (1.*rand()/RAND_MAX)
//#define _CRT_SECURE_NO_WARNINGS //MSVS issue

typedef struct {
	bool is_transmitting;
	bool was_failture;
	long long int begin_waiting;
	long long int end_time;
	long long int next_flip;
} message;

const long long int slots = 100000; //more is better but longer
const long long int ab = 4;
const float p = 1. / ab;
const int max_parts = 6;

const string dir("C:\\");
const string fname = dir + "res.txt";
const string gnuscript_path = dir + "gnu.txt";
const string path2gnu = "gnuplot"; //enter full path to gnuplot if gnuplot is not in PATH

int main() {
	srand(time(NULL));
	FILE* f;
	goto draw; //uncomment this line if you already have data so you need just to draw it
	f = fopen(fname.c_str(), "w");
	if (!f) return printf("model file not open");
	for (long long int parts = 1; parts <= max_parts; ++parts) {
		for (float intens = .05; intens <= .95; intens += .05) {
			printf("\r%2.lld parts; intens = %.2f", parts, intens);
			vector<queue<long long int>> ab_buffs;
			vector<message> transmission_info;
			for (long long int i = 0; i < ab; ++i) {
				queue<long long int> q;
				ab_buffs.push_back(q);
				message m; m.is_transmitting = false; m.was_failture = false; m.begin_waiting = -1; m.end_time = -1; m.next_flip = -1; transmission_info.push_back(m);
			}
			long long int total_success = 0, total_delay = 0, total_queue = 0;
			for (long long int t = 0; t < slots * parts; ++t) {
				for (long long int i = 0; i < ab; ++i) {
					//if msg finished its transmission
					if (transmission_info[i].is_transmitting && (transmission_info[i].end_time < t)) {
						transmission_info[i].is_transmitting = false;
						//if no errors
						if (!transmission_info[i].was_failture) {
							++total_success;
							total_delay += t - transmission_info[i].begin_waiting;
							ab_buffs[i].pop();
						}
					}
				}
				//replenish abonents' buffers in random moment of the window with given intensity
				for (long long int i = 0; i < ab; ++i) {
					if (!(t % parts)) if (random < intens / ab) ab_buffs[i].push(t + random * parts);
				}
				//counting number of currently transmitting abonents
				long long int transmitting_now = 0;
				for (long long int i = 0; i < ab; ++i) {
					transmitting_now += transmission_info[i].is_transmitting ? 1 : 0;
				}
				//conflict if >1 message is transmitting
				if (transmitting_now > 1) for (long long int i = 0; i < ab; ++i) {
					transmission_info[i].was_failture = true;
					//if abonent stops transmission immediately comment out previous line and uncomment following:
					//transmission_info[i].end_time = t;
					//transmission_info[i].is_transmitting = false;
				}
				//abonent transmits its message (probability p) if it has any
				for (long long int i = 0; i < ab; ++i) {
					if (transmission_info[i].next_flip <= t) if (!ab_buffs[i].empty() && ab_buffs[i].front() <= t) {
						transmission_info[i].next_flip = t + parts + (random - 0.5) * parts;
						if (random < p) {
							transmission_info[i].begin_waiting = ab_buffs[i].front();
							transmission_info[i].end_time = t + parts;
							transmission_info[i].is_transmitting = true;
							transmission_info[i].was_failture = false;
						}
					}
				}
				for (long long int i = 0; i < ab; ++i) {
					total_queue += ab_buffs[i].size();
				}
			}
			fprintf(f, "%lld %f %lld %lld %f %lld\n", parts, intens, total_success, total_delay / parts, 1. * total_success / slots, total_queue / parts);
		}
	}
	fclose(f);
draw:
	const char* ylabels[] = {
		"total sent",
		"total delay",
		"lambda_{out}",
		"total queue",
	};
	f = fopen(gnuscript_path.c_str(), "w");
	if (!f) return printf("gnuscript not open");
	fprintf(f, "set autoscale\n");
	fprintf(f, "set xlabel 'lambda_{in}'\n");
	for (int i = 1; i <= 4; ++i) {
		fprintf(f, "set terminal windows %d\n", i);
		fprintf(f, "set ylabel '%s'\n", ylabels[i-1]);
		fprintf(f, "plot for [i = 1:%d] '%s' using ($1 == i? $2 : 1/0):%d w lp title sprintf(\"%%d subslots\", i)\n", max_parts, fname.c_str(), i + 2);
	}
	fprintf(f, "pause mouse key\n");
	fclose(f);
	string system_message = path2gnu + " " + gnuscript_path;
	system(system_message.c_str());
	return 0;
}