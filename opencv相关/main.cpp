#include "include/Pipeline.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <iostream>
#include <fstream>
#include <string>
#include <opencv2/opencv.hpp>

using namespace std;

int main(int argc, char** argv)
{
    const char* pipePath = "/tmp/photoname_pipe";
    char imagePath[256];

    // 创建命名管道
    if (mkfifo(pipePath, 0666) == -1)
    {
        if (errno != EEXIST)
        {
            perror("mkfifo");
            exit(1);
        }
    }

    // 加载数据模型
    pr::PipelinePR prc("model/cascade.xml",
        "model/HorizonalFinemapping.prototxt",
        "model/HorizonalFinemapping.caffemodel",
        "model/Segmentation.prototxt", "model/Segmentation.caffemodel",
        "model/CharacterRecognization.prototxt",
        "model/CharacterRecognization.caffemodel",
        "model/SegmenationFree-Inception.prototxt",
        "model/SegmenationFree-Inception.caffemodel");

    cout << "等待管道数据..." << endl;

    while (true)
    {
        int fd = open(pipePath, O_RDONLY);
        if (fd == -1)
        {
            perror("open");
            exit(1);
        }

        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);

        fd_set tempfds = readfds;
        int ret = select(fd + 1, &tempfds, NULL, NULL, NULL);
        if (ret == -1)
        {
            perror("select");
            close(fd);
            exit(1);
        }

        if (FD_ISSET(fd, &tempfds))
        {
            ssize_t bytesRead = read(fd, imagePath, sizeof(imagePath) - 1);
            if (bytesRead > 0)
            {
                imagePath[bytesRead] = '\0'; // Null-terminate the string
                cout << "请等待识别过程，已读到的管道数据: " << imagePath << endl;

                // 增加调试信息
                cout << "尝试读取图片: " << imagePath << endl;

                // 等待文件完全写入
                usleep(500000); // 等待500毫秒

                // 开始计时
                clock_t startTime = clock();

                string pn;
                // 读取一张图片（支持BMP、JPG、PNG等）
                cv::Mat image = cv::imread(imagePath);
                if (image.empty())
                {
                    cout << "无法读取图片: " << imagePath << endl;
                    close(fd);
                    continue; // 跳过当前循环，继续下一次读取
                }

                std::vector<pr::PlateInfo> res;

                try
                {
                    res = prc.RunPiplineAsImage(image, pr::SEGMENTATION_FREE_METHOD);
                }
                catch (...)
                {
                    cout << "识别结束，未检测到车牌" << endl;
                    close(fd);
                    continue; // 跳过当前循环，继续下一次读取
                }

                // 检查识别时间是否超过10秒
                clock_t endTime = clock();
                double elapsedTime = double(endTime - startTime) / CLOCKS_PER_SEC;
                if (elapsedTime > 10.0)
                {
                    cout << "识别结束，超时" << endl;
                    close(fd);
                    continue; // 跳过当前循环，继续下一次读取
                }

                for (auto st : res)
                {
                    pn = st.getPlateName();
                    if (pn.length() == 9)
                    {
                        cout << "识别结束，检测到车牌:" << pn.data();
                        cout << "，确信率:" << st.confidence << endl;
                    }
                }
            }
            else if (bytesRead == 0)
            {
                cerr << "管道已关闭" << endl;
                close(fd);
                exit(1);
            }
            else
            {
                perror("read");
                close(fd);
                exit(1);
            }
        }

        close(fd);
    }

    return 0;
}