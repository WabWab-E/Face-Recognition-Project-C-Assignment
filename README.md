# Real-time Face Recognition Project

본 프로젝트는 C++와 Dlib, OpenCV 라이브러리를 사용하여 실시간 얼굴 인식(Face Recognition)을 구현한 프로그램입니다.

## 🛠 주요 기능
- 실시간 영상 스트림 내 얼굴 검출 및 인식

## ⚙️ 개발 환경
- Language C++
- IDE Visual Studio 2026
- Library Dlib, OpenCV
- OS Windows 10/11 (x64)

## 🚀 실행 방법

본 프로젝트는 `dlib.lib`를 정적 라이브러리로 빌드하여 사용합니다.

1. Dlib 빌드 - Dlib 소스 코드에서 `dliballsource.cpp`를 포함하는 정적 라이브러리 프로젝트를 생성합니다.
   - 프로젝트 속성에서 `DLIB_PNG_SUPPORT`, `DLIB_JPEG_SUPPORT`, `_CRT_SECURE_NO_WARNINGS` 매크로를 정의합니다.
   - `Release` `x64` 모드로 빌드하여 `dlib.lib`를 생성합니다.

2. 프로젝트 연동
   - 프로젝트 속성  링커  일반  [추가 라이브러리 디렉터리]에 `dlib.lib` 폴더 경로를 추가합니다.
   - 프로젝트 속성  링커  입력  [추가 종속성]에 `dlib.lib`를 기입합니다.
   - [추가 포함 디렉터리]에 Dlib 최상위 폴더 경로를 추가합니다.

3. 전처리기 설정
   - 메인 프로젝트의 속성에서도 빌드 시 적용했던 동일한 매크로(`DLIB_PNG_SUPPORT`, `DLIB_JPEG_SUPPORT`, `_CRT_SECURE_NO_WARNINGS`)를 반드시 정의해야 합니다.

## 📁 폴더 구조
```text
project-root
├── include            # Dlib 소스 파일 (dlib-20.0 폴더 내용물)
│    └── dlib          # Dlib 핵심 라이브러리 소스, libpng, libjpeg 등 외부 의존성 소스
├── lib
│    └── dlib.lib       # 빌드 완료된 정적 라이브러리 파일
└── src
     └── main.cpp       # 메인 소스 코드
```

## 📝 참고 사항
- 용량 최적화를 위해 .lib 및 빌드 결과물은 저장소에서 제외되었습니다.
