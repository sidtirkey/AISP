#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int collect_cpu_metric(char *cpu, unsigned long *total_cpu_time, unsigned long *idle_time){
	/*Collects CPU metric from /proc/stat file*/
	FILE *fp = fopen("/proc/stat","r");
	if (fp == NULL){
		perror("Error opening /proc/stat\n");
		exit(1);
	}
	unsigned long user, nice, system, idle, iowait, irq, softirq;
	unsigned long steal, guest, guest_nice;
	unsigned long sum_cpu_time;
	int parse_result = fscanf(fp,"%s %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu", cpu, &user,
			&nice, &system, &idle, &iowait, &irq, &softirq, &steal, &guest, &guest_nice);
	if (parse_result == 11){
		sum_cpu_time = user+nice+system+idle+iowait+irq+softirq+steal;
		*total_cpu_time = sum_cpu_time;
		*idle_time = idle;
	}
	if (parse_result == 0){
		fprintf(stderr, "CPU metric collection failed because of parsing error\n");
		exit(1);
	}
	if (parse_result == -1){
		fprintf(stderr, "Input failure, reached EOF before reaching any file\n");
		exit(1);
	}
	fclose(fp);
	return 0;
}


double cpu_util_interval(int sleep_time){
	/*returns cpu utilisation in terms of a percentage in double */
	char cpu[10];
	unsigned long total_cpu_util_time_1, idle_time_1, total_cpu_util_time_2, idle_time_2;
	unsigned long delta_total_cpu_time, delta_idle;
	double cpu_perc;
	collect_cpu_metric(cpu, &total_cpu_util_time_1, &idle_time_1);
	usleep(sleep_time * 1000);
	collect_cpu_metric(cpu, &total_cpu_util_time_2, &idle_time_2);
	delta_total_cpu_time = total_cpu_util_time_2 - total_cpu_util_time_1;
	delta_idle = idle_time_2 - idle_time_1;
	cpu_perc = (1.0 - (double)delta_idle/(double)delta_total_cpu_time)*100.0;
	return cpu_perc;
}


int main(){
	while(1){
		printf("CPU:%f%%\n", cpu_util_interval(1000));
	}
	return 0;
}


