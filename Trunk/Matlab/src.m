close all; clear all; clc;

tx = 0:1:12000;
x = sin(2*pi*250/44100*tx);

[r,b] = resample(x,160,441);
%%
%plot(tx,x,'+-',ty,y,'o:')
%legend('original','resampled');

figure; plot(x,'r'); hold on; plot(r, 'b');




m = length(x);
u = [x; zeros(159, m)];
u = reshape(u, m*160, 1);


y = filter(b,1,u);


n = length(y);
n = floor(n / 441);
y = y(1:n*441);
y = reshape(y, 441, n);
y = y(1,:);


plot(y(11:end), 'k');

figure; plot(r(1:4000) - y(11:4010));