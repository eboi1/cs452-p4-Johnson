# CS452 P4 - Eric Johnson

Steps to configure, build, run, and test the project.

## Building

```bash
make
```

## Testing

```bash
make check
```

I also ran command line tests to ensure that my implementation was correct according to this usage: 
```bash
./myprogram -c <num_consumers> -p <num_producers> -i <num_items> -s <queue_size> [-d]
```

For example, this input: 
```bash
./myprogram -c 2 -p 3 -i 10 -s 5 -d
```
Produced this output: 
```bash
Simulating 3 producers 2 consumers with 3 items per thread and a queue size of 5
Creating 2 consumer threads
Queue is empty:true
Total produced:9
Total consumed:9
 3.989990 9 
 ```

## Clean

```bash
make clean
```

## Install Dependencies

In order to use git send-mail you need to run the following command:

```bash
make install-deps
```
