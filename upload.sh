#!/bin/bash

# FTP 서버 주소 및 사용자 정보
HOST='ftp.example.com'
USER='username'
PASS='password'

# 업로드할 파일명
LOCAL_FILE='example.txt'
REMOTE_FILE='example.txt'

# FTP 명령 실행
ftp -inv $HOST <<EOF
user $USER $PASS
put $LOCAL_FILE $REMOTE_FILE
bye
EOF

echo "파일 업로드 완료!"
