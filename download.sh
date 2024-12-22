#!/bin/bash

# FTP 서버 주소 및 사용자 정보
HOST='ftp.example.com'
USER='username'
PASS='password'

# 다운로드할 파일명
REMOTE_FILE='example.txt'
LOCAL_FILE='downloaded_example.txt'

# FTP 명령 실행
ftp -inv $HOST <<EOF
user $USER $PASS
get $REMOTE_FILE $LOCAL_FILE
bye
EOF

echo "파일 다운로드 완료!"
