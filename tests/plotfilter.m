fin = fopen('filein.bin');
fout = fopen('fileout.bin');

pdattype = fread(fout, 1, 'uint32');
ptype = 'float32';
if pdattype == 1
    ptype = 'float64';
end
numlen = fread(fout, 1, 'uint32');
num = fread(fout, numlen, ptype);
denumlen = fread(fout, 1, 'uint32');
denum = fread(fout, denumlen, ptype);

dattype = fread(fin, 1, 'uint32');
nchann = fread(fin, 1, 'uint32');
dattype2 = fread(fout, 1, 'uint32');
nchann2 = fread(fout, 1, 'uint32');

if (dattype ~= dattype2) || (nchann ~= nchann2)
    error('data params differs in the 2 files');
end

type = 'float32';
if dattype == 1
    type = 'float64';
end



datin = fread(fin,[nchann,inf],type);
datout = fread(fout,[nchann,inf],type);
datmatlab = filter(num,denum,datin,[],2);

fclose(fin);
fclose(fout);

for ichann=1:6:nchann
    figure
    plot(datin(ichann,:),'r')
    hold on
    plot(datout(ichann,:),'b')
    plot(datmatlab(ichann,:),'g--')
    hold off
    axis([0 600 -2 2])
end

nchann
diffval = (datmatlab - datout);
[errval,imax] = max(abs(diffval),[],2);
