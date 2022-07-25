# ProgettoKernelProgramming1
Progetto del corso di Kernel Programming 1


Istruzioni per la creazione del file system per i test:
http://retis.santannapisa.it/luca/KernelProgramming/build_and_update_tinyfs.txt


Per compilare il modulo:
$ KERNEL_DIR=/home/diego/KernelProgramming1/linux-5.9.9 make

Per testare il modulo dentro QEMU:
Caricare il modulo:
sudo /sbin/insmod modulo.ko

Per elencare i moduli caricati:
sudo /sbin/lsmod

Rimuovere il modulo:
sudo /sbin/insmod modulo
