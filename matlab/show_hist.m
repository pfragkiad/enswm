%put the difference with the theoretical duration value (simulated time)

subplot(3,2,1)
data = load('durations_500.txt');
data = sort(data);
data = (data(10:end-10) -20000)/20000;
h = histogram(data);
xlabel('ÄTime/(Search Time Interval) [%]')
xlim([-20 20])
ylim([0 20000])
title('Speedup 500 (10 sec -> 20 ms )')

subplot(3,2,3)
data = load('durations_1000.txt');
data = sort(data);
data = (data(10:end-10)-10000)/10000;
h = histogram(data);
xlabel('ÄTime/(Search Time Interval) [%]')
xlim([-20 20])
ylim([0 20000])
title('Speedup 1000 (10 sec -> 10 ms)')


subplot(3,2,5)
data = load('durations_10000.txt');
data = sort(data);
data = (data(20:end-20)-1000)/1000*100;
h = histogram(data);
xlabel('ÄTime/(Search Time Interval) [%]')
xlim([-20 20])
ylim([0 30000])
title('Speedup 10000 (10 sec -> 1 ms)')

subplot(3,2,2)
data = load('durations_500.txt');
data = sort(data);
data = data(10:end-10) -20000;
h = histogram(data);
xlabel('Time Difference (ms)')
xlim([-1000 1000])
ylim([0 20000])
title('Speedup 500 (10 sec -> 20 ms )')

subplot(3,2,4)
data = load('durations_1000.txt');
data = sort(data);
data = data(10:end-10)-10000;
h = histogram(data);
xlabel('Time Difference (ms)')
xlim([-1000 1000])
ylim([0 20000])
title('Speedup 1000 (10 sec -> 10 ms)')


subplot(3,2,6)
data = load('durations_10000.txt');
data = sort(data);
data = data(20:end-20)-1000;
h = histogram(data);
xlabel('Time Difference (ms)')
xlim([-1000 1000])
ylim([0 30000])
title('Speedup 10000 (10 sec -> 1 ms)')
