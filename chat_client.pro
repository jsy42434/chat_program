# Qt 프로젝트 파일

QT += core gui network widgets  # 사용 모듈 (network는 QTcpSocket 사용에 필요)
CONFIG += c++11                # C++11 표준 지원

# 프로젝트의 타겟 이름
TARGET = chat_client
TEMPLATE = app

# 소스 파일 지정
SOURCES += chat_client.cpp

# 헤더 파일 지정 (필요한 경우 추가)
HEADERS +=

# 기타 설정
