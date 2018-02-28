td "tDkm"
unld "dkm.out"
copy("/bd0:1/var/lib/iot/update/dkm.out","/bd0:1/bin/dkm.out")
cd "/bd0:1/bin"
ld < dkm.out
taskSpawn("tDkm",100,0,0x1000,start,0,0,0,0,0,0,0)
