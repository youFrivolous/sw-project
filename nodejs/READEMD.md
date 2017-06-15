## Node.js Server

### Structure

```
client/
server/
  | images/    <-- image store
nodejs/
  |- app.js    <-- run this
  |- package.json
  |- routes/
  |- views/
```

### Run server

```
$ node app.js
```

### Description

소켓 프로그램으로 전송되어 저장된 이미지들(server/images/)을 Node.js 서버를 통해 출력합니다.