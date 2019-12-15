# README

## 编译

```shell
make

# bench
wrk2 -t2 -c100 -d10s -R2000 http://127.0.0.1:7000
wrk2 -t2 -c100 -d10s -R2000 http://127.0.0.1:5432
wrk2 -t2 -c100 -d10s -R2000 --latency http://127.0.0.1:7000
wrk2 -t2 -c100 -d10s -R2000 --latency http://127.0.0.1:5432

while true; do curl localhost:7000; done
while true; do curl localhost:5432; done
```
