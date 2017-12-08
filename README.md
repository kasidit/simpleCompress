# simpleCompress
<h1>Simple Compression and Data Transfer programs between two machines.</h1>
<p><p>
This program is written to test simple data compression and data transfer relationship. 
It uses compress() and uncompress() of zlib library to compress and uncompress data, respectively. 
To use it, you have to do as follows.
<p><p>
<h2>1. Compile the program</h2>
I assume you have zlib installed on your machine. 
<pre>
$ cd simpleCompress
$ make
</pre>
<p><p>
<h2>2. Create a 250MB file containing random numbers</h2>
<p><p>
<pre>
$ ./zzz data
$ ls 
... datafile
$
</pre>
<p><p>
<h2>3. Do compression and then uncompress the data</h2>
<p><p>
You should see timing reports of the compression and uncompression operation
<pre>
$ ./zzz compd
></pre>
<p><p>
<h2>4. Do compression and then trasfer result to detination and uncompress the data</h2>
<p><p>You have to modify the zzz.c and change IP address of the server machine. Copy zzz.c to 
a client and server computers. Compile the program on both machines, and then run them. 
<p><p>
<b>On server machine:</b><br>
<pre>
$ ./zzz serd
</pre>
r<b>Then on the client run:</b>
<pre>

$./zzz clid
</pre>
See and use the timing reports as you wish. 
<p><p>
<h2>5. Trasfer plain data across machines</h2>
<p><p>
<b>On server machine:</b><br>
<<pre>
$ ./zzz sern

</pre>
<b>On the client:</b>
<pre>
$./zzz clin
</pre>
<p><p>
<h2>6. Compress a file and compress and transfer to a destination host</h2>
<p><p>
I have written another program that will take any file and transfer to any destination. It's the zzz2.c file. 
You have to make a copy of this file on the source and destination computer, and then compile it. 
<pre>
$ make zzz2
</pre>
<b>On server computer (192.168.8.12):</b>
<pre>
$ wget http://sun.aei.polsl.pl/~sdeor/corpus/mozilla.bz2
$ bunzip2 mozilla.bz2
$ ./zzz2 serd mozilla 192.168.8.12 5050
</pre>
<b>On client computer:</b>
<pre>
$ wget http://sun.aei.polsl.pl/~sdeor/corpus/mozilla.bz2
$ bunzip2 mozilla.bz2
$ ./zzz2 clid mozilla 192.168.8.12 5050
</pre>
<p><p>
<h2>7. Just transfer data</h2>
<p><p>
<b>On server computer (192.168.8.12):</b>
<pre>
$ ./zzz2 sern mozilla 127.0.0.1 5050
</pre>
<b>On client computer:</b>
<pre>
$ ./zzz2 clin mozilla 192.168.8.12 5050
</pre>
<h2>8. Transfer data over an lz4 compression tunnel (using netcat)</h2>
<p><p>
<b>On server computer (192.168.8.12):</b><br>
terminal 1<br>
<pre>
$ ./zzz2 sern mozilla 127.0.0.1 5050
</pre>
terminal 2<br>
<pre>
nc -l 5503 | lz4 -qq -d -c - | nc 127.0.0.1 5050
</pre>
<b>On client computer:</b><br>
terminal 1<br>
<pre>
$ nc -l 4403 | lz4 -qq -1 -B4 -c - | nc 192.168.8.12 5503
</pre>
terminal 2<br>
<pre>
$ ./zzz2 clin mozilla 127.0.0.1 4403
</pre>
