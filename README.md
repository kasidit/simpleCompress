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
p><p>
<h2>2. Do compression and then uncompress the data</h2>
<p><p>
You should see timing reports of the compression and uncompression operation
<pre>
$ ./zzz compd
</pre>
p><p>
<h2>3. Do compression and then trasfer result to detination and uncompress the data</h2>
<p><p>
You have to modify the zzz.c and change IP address of the server machine. Copy zzz.c to 
a client and server computers. Compile the program on both machines, and then run them. 
<p><p>
<b>On server machine:</b><br>
<pre>
$ ./zzz serd
</pre>
<b>Then on the client run:</b>
<pre>
$./zzz clid
</pre>
See and use the timing reports as you wish. 
p><p>
<h2>3. Trasfer plain data across machines</h2>
<p><p>
<b>On server machine:</b><br>
<pre>
$ ./zzz sern
</pre>
<b>On the client:</b>
<pre>
$./zzz clin
</pre>
