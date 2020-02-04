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

const long long total = 100000;
const long long ab = 3;
const long long W = 2 * ab;
const long long max_cps = 6;

const string dir("C:\\");
const string fname = dir + "res.txt";
const string gnuscript_path = dir + "gnu.txt";
const string path2gnu = "gnuplot"; //enter full path to gnuplot if gnuplot is not in PATH

int main() {
	srand(time(NULL));
	FILE* f;
	//goto draw; //uncomment this line if you already have data so you need just to draw it
	f = fopen(fname.c_str(), "w");
	if (!f) return printf("model file not open");
	for (long long cps = 1; cps < max_cps; ++cps) {
		for (float intens = .1; intens < .95; intens += .01) {
			printf("Copies = %lld; intens = %f\r", cps, intens);
			long long t = 0;
			long long total_success = 0;
			long long total_delay = 0, total_queue = 0;
			vector<queue<long long>> ab_buffs;
			for (long long i = 0; i < ab; ++i) {
				queue<long long> q; ab_buffs.push_back(q);
			}
			while (t < total) {
				for (int i = 0; i < ab; ++i) for (long long j = 0; j < W; ++j)
					if (random * 1. < intens / ab) ab_buffs[i].push(t + j);
				vector<pair<long long, long long>>this_window(W, make_pair(0, -1));
				for (long long i = 0; i < ab; ++i) {
					if (ab_buffs[i].empty()) continue;
					vector<bool> cur(W, false);
					for (long long j = 0; j < cps;) {
						long long curr = rand() % W;
						if (cur[curr]) continue;
						cur[curr] = true;
						this_window[curr].first++;
						this_window[curr].second = i;
						++j;
					}
				}
				t += W;
				vector<bool> sent(ab, false);
				for (long long i = 0; i < W; ++i) {
					if (this_window[i].first == 1 && \
						!sent[this_window[i].second]) {
						long long cur_ab = this_window[i].second;
						sent[cur_ab] = true;
						ab_buffs[cur_ab].pop();
						total_success++;
					}
				}
				for (long long i = 0; i < ab; ++i) {
					total_queue += ab_buffs[i].size();
					if (!ab_buffs[i].empty()) total_delay += \
						t - ab_buffs[i].front();
				}
			}
			fprintf(f, "%lld %lld %f %f %f %lld\n", cps, ab, intens, 1. * total_queue /
				total, 1. * total_success / total, total_delay / total_success);
		}
		fprintf(f, "\n");
	}
	fclose(f);
draw:
	const char* ylabels[] = {
		"queue",
		"lambda_{out}",
		"delay",
	};
	f = fopen(gnuscript_path.c_str(), "w");
	if (!f) return printf("gnuscript not open");
	fprintf(f, "set autoscale\n");
	fprintf(f, "set xlabel 'lambda_{in}'\n");
	for (int i = 1; i <= 3; ++i) {
		fprintf(f, "set terminal windows %d\n", i);
		fprintf(f, "set ylabel '%s'\n", ylabels[i-1]);
		fprintf(f, "plot for [i = 1:%lld] '%s' using ($1 == i? $3 : 1/0):%d w lp title sprintf(\"%%d copies\", i)\n", max_cps - 1, fname.c_str(), i + 3);
	}
	fprintf(f, "pause mouse key\n");
	fclose(f);
	string system_message = path2gnu + " " + gnuscript_path;
	system(system_message.c_str());
	return 0;
}