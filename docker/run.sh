docker stop sip
docker rm sip  
docker build -t sip .
docker run --cap-add SYS_NICE  --security-opt seccomp:unconfined -p 5060:5060 --name="sip"  -d sip
docker exec -it sip bash
