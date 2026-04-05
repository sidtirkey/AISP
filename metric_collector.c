#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


int collect_cpu_metric(char *cpu, unsigned long *total_cpu_time, unsigned long *idle_time){
	/*Collects CPU metric from /proc/stat file*/
	FILE *fp = fopen("/proc/stat","r");
	if (fp == NULL){
		perror("Error opening /proc/stat");
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
		return -1;
	}
	if (parse_result == -1){
		fprintf(stderr, "Input failure, reached EOF before reaching any file\n");
		return -1;
	}

	fclose(fp);
	return 0;
}

int collect_memory_metric(unsigned long *mem_tot, unsigned long *mem_avail){
	/*Parses /proc/meminfo and assigns MemTotal: and MemAvailable: field */
	FILE *fp = fopen("/proc/meminfo", "r");
	if (fp == NULL){
		perror("Unable to open memory info file");
		exit(1);
	}
	char label[64];
	char unit[10];
	unsigned long value;
	*mem_tot = 0;
	*mem_avail = 0;
	while (fscanf(fp, "%63s %lu %9s", label, &value, unit) == 3){
		if (strcmp(label,"MemTotal:") == 0){
			*mem_tot = value;
		}
		else if (strcmp(label, "MemAvailable:") == 0){
			*mem_avail = value;
			break;
		}
	}
	fclose(fp);
	return 0;
}


double cpu_util_interval(int sleep_time){
	/*returns cpu utilisation in terms of a percentage in double */
	char cpu[10];
	unsigned long total_cpu_util_time_1, idle_time_1, total_cpu_util_time_2, idle_time_2;
	unsigned long delta_total_cpu_time, delta_idle;
	double cpu_perc = 0.0;
	int coll_cpu1 = collect_cpu_metric(cpu, &total_cpu_util_time_1, &idle_time_1);
	if (coll_cpu1 == -1){
		return -1;
	}
	usleep(sleep_time * 1000);
	int coll_cpu2 = collect_cpu_metric(cpu, &total_cpu_util_time_2, &idle_time_2);
	if (coll_cpu2 == -1){
		return -1;
	}
	delta_total_cpu_time = total_cpu_util_time_2 - total_cpu_util_time_1;
	if (delta_total_cpu_time == 0){
		return 0.0;
	}
	delta_idle = idle_time_2 - idle_time_1;
	cpu_perc = (1.0 - (double)delta_idle/(double)delta_total_cpu_time)*100.0;
	return cpu_perc;
}


double memory_util(){
	/*returns used memory in KiloBytes */
	unsigned long mem_tot, mem_avail;
	mem_tot = 0;
	mem_avail = 0;
	int coll_mem = collect_memory_metric(&mem_tot, &mem_avail);
	if (coll_mem == -1){
		return -1;
	}
	double used_mem = mem_tot - mem_avail;
	return used_mem;
}



int main(){
	printf("Memory Util: %f\n", memory_util());
	while (1){
		printf("CPU util: %f\n", cpu_util_interval(500));
	}
	return 0;
}


