sz=1048576;
fd=fopen('ggtest','r');
[d,c]=fread(fd,[1 sz],'float32');
fclose(fd);
pwrdev = sum(d.^2)/sz
pwrdevdb = 10*log10(pwrdev)
f=fft(d);
fdiv=128;
ff=zeros(1,sz/fdiv);
for t=1:sz/fdiv,ff(t)=sum(abs(f(t*fdiv-fdiv+1:t*fdiv)).^2);end;
plot(ff(1:round(sz/fdiv/80))/c^2);
