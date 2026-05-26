#include <iostream>
#include <string>
#include <vector>

#include <opencv2/opencv.hpp>
#include <opencv2/core/utils/logger.hpp>

#include <dlib/dnn.h>
#include <dlib/image_processing.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/opencv.h>

using std::cerr;
using std::cin;
using std::cout;
using std::endl;
using std::string;

using namespace dlib;

// ==========================================================
// Configuration
// ==========================================================

constexpr int FRAME_WIDTH = 640;
constexpr int FRAME_HEIGHT = 480;
constexpr int DETECTION_INTERVAL = 5;
constexpr double RECOGNITION_THRESHOLD = 0.6;

constexpr char WINDOW_NAME[] = "Real-time Face Recognition";

// ==========================================================
// Dlib Face Recognition Network
// ==========================================================

template <template <int, template<typename> class, int, typename> class block,
    int N,
    template<typename> class BN,
    typename SUBNET>
using residual = add_prev1<block<N, BN, 1, tag1<SUBNET>>>;

template <template <int, template<typename> class, int, typename> class block,
    int N,
    template<typename> class BN,
    typename SUBNET>
using residual_down =
add_prev2<avg_pool<2, 2, 2, 2,
    skip1<tag2<block<N, BN, 2, tag1<SUBNET>>>>>>;

template <int N,
    template<typename> class BN,
    int stride,
    typename SUBNET>
using block =
BN<con<N, 3, 3, 1, 1,
    relu<BN<con<N, 3, 3, stride, stride, SUBNET>>>>>;

template <int N, typename SUBNET>
using ares = relu<residual<block, N, affine, SUBNET>>;

template <int N, typename SUBNET>
using ares_down = relu<residual_down<block, N, affine, SUBNET>>;

template <typename SUBNET>
using alevel0 = ares_down<256, SUBNET>;

template <typename SUBNET>
using alevel1 =
ares<256,
    ares<256,
    ares_down<256, SUBNET>>>;

template <typename SUBNET>
using alevel2 =
ares<128,
    ares<128,
    ares_down<128, SUBNET>>>;

template <typename SUBNET>
using alevel3 =
ares<64,
    ares<64,
    ares<64,
    ares_down<64, SUBNET>>>>;

template <typename SUBNET>
using alevel4 =
ares<32,
    ares<32,
    ares<32, SUBNET>>>;

using FaceRecognitionNet = loss_metric<
    fc_no_bias<128,
    avg_pool_everything<
    alevel0<
    alevel1<
    alevel2<
    alevel3<
    alevel4<
    max_pool<3, 3, 2, 2,
    relu<affine<con<32, 7, 7, 2, 2,
    input_rgb_image_sized<150>
    >>>>>>>>>>>>;

// ==========================================================
// Data Structure
// ==========================================================

struct UserFaceData {
    string name;
    matrix<float, 0, 1> face_descriptor;
};

// ==========================================================
// Utility Functions
// ==========================================================

string recognizeFace(
    const matrix<float, 0, 1>& descriptor,
    const std::vector<UserFaceData>& known_faces) {

    string recognized_name = "Unknown";
    double min_distance = RECOGNITION_THRESHOLD;

    for (const auto& known_face : known_faces) {
        const double distance =
            length(descriptor - known_face.face_descriptor);

        if (distance < min_distance) {
            min_distance = distance;
            recognized_name = known_face.name;
        }
    }

    return recognized_name;
}

void registerFace(
    const matrix<float, 0, 1>& descriptor,
    std::vector<UserFaceData>& known_faces) {

    string new_name;

    cout << "\n[등록 대상 감지] "
        << "콘솔창에 등록할 이름을 입력하세요: ";

    cin >> new_name;

    known_faces.push_back({
        new_name,
        descriptor
        });

    cout << ">> '" << new_name
        << "'님이 데이터베이스에 등록되었습니다.\n"
        << endl;
}

void printProgramGuide() {
    cout << "==================================================\n";
    cout << " [화면 최적화] 실시간 C++ 얼굴 인식 시스템 구동 중 \n";
    cout << " [N] 키: 현재 화면의 얼굴 등록\n";
    cout << " [ESC] 키: 프로그램 종료\n";
    cout << "==================================================\n";
}

// ==========================================================
// Main
// ==========================================================

int main() {
    cv::utils::logging::setLogLevel(
        cv::utils::logging::LOG_LEVEL_ERROR
    );

    try {
        frontal_face_detector detector =
            get_frontal_face_detector();

        shape_predictor shape_predictor_model;
        deserialize("shape_predictor_5_face_landmarks.dat")
            >> shape_predictor_model;

        FaceRecognitionNet recognition_network;
        deserialize("dlib_face_recognition_resnet_model_v1.dat")
            >> recognition_network;

        std::vector<UserFaceData> known_faces;

        bool is_register_mode = false;

        cv::VideoCapture cap(0, cv::CAP_MSMF);

        if (!cap.isOpened()) {
            cap.open(0);

            if (!cap.isOpened()) {
                cerr << "카메라 장치를 인식할 수 없습니다!"
                    << endl;

                return EXIT_FAILURE;
            }
        }

        cap.set(cv::CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);

        cv::namedWindow(
            WINDOW_NAME,
            cv::WINDOW_AUTOSIZE
        );

        printProgramGuide();

        int frame_count = 0;

        std::vector<rectangle> faces;
        std::vector<string> face_names;

        cv::Mat frame;

        while (cap.read(frame)) {
            if (frame.empty()) {
                cv::waitKey(1);
                continue;
            }

            if (frame_count % DETECTION_INTERVAL == 0) {
                cv_image<bgr_pixel> cimg(frame);

                faces = detector(cimg);

                face_names.clear();

                for (const auto& face : faces) {
                    const auto shape =
                        shape_predictor_model(cimg, face);

                    matrix<rgb_pixel> face_chip;

                    extract_image_chip(
                        cimg,
                        get_face_chip_details(shape, 150, 0.25),
                        face_chip
                    );

                    const auto face_descriptor =
                        recognition_network(face_chip);

                    const string recognized_name =
                        recognizeFace(
                            face_descriptor,
                            known_faces
                        );

                    face_names.push_back(recognized_name);

                    if (is_register_mode) {
                        registerFace(
                            face_descriptor,
                            known_faces
                        );

                        is_register_mode = false;
                    }
                }
            }

            ++frame_count;

            for (size_t i = 0; i < faces.size(); ++i) {
                cv::Rect face_rect(
                    faces[i].left(),
                    faces[i].top(),
                    faces[i].width(),
                    faces[i].height()
                );

                face_rect &=
                    cv::Rect(0, 0, frame.cols, frame.rows);

                cv::rectangle(
                    frame,
                    face_rect,
                    cv::Scalar(0, 255, 0),
                    2
                );

                if (i < face_names.size()) {
                    cv::putText(
                        frame,
                        face_names[i],
                        cv::Point(
                            face_rect.x,
                            std::max(15, face_rect.y - 10)
                        ),
                        cv::FONT_HERSHEY_SIMPLEX,
                        0.6,
                        cv::Scalar(255, 255, 255),
                        2
                    );
                }
            }

            cv::imshow(WINDOW_NAME, frame);

            const int key = cv::waitKey(1);

            if (key == 27) {
                break;
            }

            if (key == 'n' || key == 'N') {
                is_register_mode = true;

                cout << "\n다음 프레임의 얼굴을 캡처합니다. "
                    << "콘솔창을 확인하세요."
                    << endl;
            }
        }

        cv::destroyAllWindows();
    }
    catch (const std::exception& exception) {
        cerr << "\n런타임 예외 발생: "
            << exception.what()
            << endl;

        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}