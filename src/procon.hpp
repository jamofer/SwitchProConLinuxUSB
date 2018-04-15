#include "hidapi.h"
#include <stdio.h>
#include <array>
#include <cstring>
#include <unistd.h>
#include <chrono>
#include <ctime>
#include <ratio>
#include <linux/uinput.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
//#include <optional>

#define PROCON_DRIVER_VERSION "0.4"

#define KNRM "\x1B[0m"
#define KRED "\x1B[31m"
#define KGRN "\x1B[32m"
#define KYEL "\x1B[33m"
#define KBLU "\x1B[34m"
#define KMAG "\x1B[35m"
#define KCYN "\x1B[36m"
#define KWHT "\x1B[37m"

#define PROCON_ID 0x2009
#define NINTENDO_ID 0x057E

#define MAX_N_CONTROLLERS 4

class ProController
{

    enum BUTTONS
    {
        d_left,
        d_right,
        d_up,
        d_down,
        A,
        B,
        X,
        Y,
        plus,
        minus,
        home,
        share,
        L1,
        L2,
        L3,
        R1,
        R2,
        R3,
        None
    };

    using uchar = unsigned char;
    static constexpr uchar ledCommand{0x30};
    static constexpr uchar get_input{0x1f};
    static constexpr uchar center{0x7e};
    static constexpr size_t exchange_length{0x400};
    using exchange_array = std::array<uchar, exchange_length>; // might need std::optional<>
                                                               // static constexpr size_t buflen{ 80 };
                                                               // std::array<char, buflen> buffer;

  public:
    //ProController() {}
    //~ProController() {}

    static const uchar bit_position(ProController::BUTTONS button)
    {
        switch (button)
        {
        case d_left:
            return 0x04;
            break;
        case d_right:
            return 0x03;
            break;
        case d_up:
            return 0x02;
            break;
        case d_down:
            return 0x01;
            break;
        case A:
            return 0x04;
            break;
        case B:
            return 0x03;
            break;
        case X:
            return 0x02;
            break;
        case Y:
            return 0x01;
            break;
        case plus:
            return 0x02;
            break;
        case minus:
            return 0x01;
            break;
        case home:
            return 0x05;
            break;
        case share:
            return 0x06;
            break;
        case L1:
            return 0x07;
            break;
        case L2:
            return 0x08;
            break;
        case L3:
            return 0x04;
            break;
        case R1:
            return 0x07;
            break;
        case R2:
            return 0x08;
            break;
        case R3:
            return 0x03;
            break;
        case None:
            return 0x00;
            break;
        default:
            red();
            printf("ERROR: Tried to find bitpos of unknown button!\n");
            normal();
            return 0x00;
            break;
        }
    }

    static const uchar byte_button_value(ProController::BUTTONS button)
    {
        switch (button)
        {
        case d_left:
            return 0x08;
            break;
        case d_right:
            return 0x04;
            break;
        case d_up:
            return 0x02;
            break;
        case d_down:
            return 0x01;
            break;
        case A:
            return 0x08;
            break;
        case B:
            return 0x04;
            break;
        case X:
            return 0x02;
            break;
        case Y:
            return 0x01;
            break;
        case plus:
            return 0x02;
            break;
        case minus:
            return 0x01;
            break;
        case home:
            return 0x10;
            break;
        case share:
            return 0x20;
            break;
        case L1:
            return 0x40;
            break;
        case L2:
            return 0x80;
            break;
        case L3:
            return 0x08;
            break;
        case R1:
            return 0x40;
            break;
        case R2:
            return 0x80;
            break;
        case R3:
            return 0x04;
            break;
        case None:
            return 0x00;
            break;
        default:
            red();
            printf("ERROR: Tried to find bitpos of unknown button!\n");
            normal();
            return 0x00;
            break;
        }
    }

    static const uchar data_address(ProController::BUTTONS button)
    {
        switch (button)
        {
        case d_left:
            return 0x0f;
            break;
        case d_right:
            return 0x0f;
            break;
        case d_up:
            return 0x0f;
            break;
        case d_down:
            return 0x0f;
            break;
        case A:
            return 0x0d;
            break;
        case B:
            return 0x0d;
            break;
        case X:
            return 0x0d;
            break;
        case Y:
            return 0x0d;
            break;
        case plus:
            return 0x0e;
            break;
        case minus:
            return 0x0e;
            break;
        case home:
            return 0x0e;
            break;
        case share:
            return 0x0e;
            break;
        case L1:
            return 0x0f;
            break;
        case L2:
            return 0x0f;
            break;
        case L3:
            return 0x0e;
            break;
        case R1:
            return 0x0d;
            break;
        case R2:
            return 0x0d;
            break;
        case R3:
            return 0x0e;
            break;
        case None:
            return 0x00;
            break;
        default:
            red();
            printf("ERROR: Tried to find data adress of unknown button!\n");
            normal();
            return 0x00;
            break;
        }
    }

    void timer()
    {
        // using namespace std::chrono;

        // high_resolution_clock::time_point t2 = high_resolution_clock::now();

        // duration<double, std::milli> time_span = t2 - t1;

        // high_resolution_clock::time_point t1 = t2;

        // printf("Time since last call ms: %u\n", time_span.count());
        // //std::cout << "It took me " << time_span.count() << " milliseconds.";
        // //std::cout << std::endl;

        using namespace std;
        clock_t now = clock();

        double elapsed_secs = double(now - last_time) / CLOCKS_PER_SEC;

        last_time = now;

        printf("Time for last %u polls: %f seconds\n", n_print_cycle, elapsed_secs);
        printf("Bad 0x00: %u\nBad 0x30: %u\n\n", n_bad_data_thirty, n_bad_data_zero);

        print_cycle_counter = 0;
        n_bad_data_thirty = 0;
        n_bad_data_zero = 0;
    }

    template <size_t length>
    exchange_array send_command(uchar command, std::array<uchar, length> const &data)
    {
        std::array<uchar, length + 0x9> buffer;
        buffer.fill(0);
        buffer[0x0] = 0x80;
        buffer[0x1] = 0x92;
        buffer[0x3] = 0x31;
        buffer[0x8] = command;
        if (length > 0)
        {
            memcpy(buffer.data() + 0x9, data.data(), length);
        }
        return exchange(buffer);
    }

    template <size_t length>
    exchange_array exchange(std::array<uchar, length> const &data, bool timed = false, int *status = nullptr)
    {

        if (!controller_ptr)
        {
            red();
            printf("ERROR: controller_ptr is nullptr!\n");
            normal();
            return {};
        }

        if (hid_write(controller_ptr, data.data(), length) < 0)
        {
            red();
            printf("ERROR: read() returned -1!\n");
            normal();
            return {};
        }

        std::array<uchar, exchange_length> ret;
        ret.fill(0);
        if (!timed)
            hid_read(controller_ptr, ret.data(), exchange_length);
        else
        {

            if (hid_read_timeout(controller_ptr, ret.data(), exchange_length, 100) == 0)
            {
                //failed to read!
                if (status)
                {
                    *status = -1;
                    return {};
                }
            }
        }
        if (status)
        {
            *status = 0;
        }
        return ret;
    }

    template <size_t length>
    exchange_array send_subcommand(uchar command, uchar subcommand, std::array<uchar, length> const &data)
    {
        std::array<uchar, length + 10> buffer{
            static_cast<uchar>(rumble_counter++ & 0xF),
            0x00,
            0x01,
            0x40,
            0x40,
            0x00,
            0x01,
            0x40,
            0x40,
            subcommand};
        if (length > 0)
        {
            memcpy(buffer.data() + 10, data.data(), length);
        }
        return send_command(command, buffer);
    }

    // void start_uinput()
    // {
    //     open()
    // }

    // void emit(int fd, int type, int code, int val)
    // {
    //     struct input_event ie;

    //     ie.type = type;
    //     ie.code = code;
    //     ie.value = val;
    //     /* timestamp values below are ignored */
    //     ie.time.tv_sec = 0;
    //     ie.time.tv_usec = 0;

    //     write(fd, &ie, sizeof(ie));
    // }

    // int u_setup()
    // {
    //     struct uinput_setup usetup;

    //     int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);

    //     /*
    // * The ioctls below will enable the device that is about to be
    // * created, to pass key events, in this case the space key.
    // */
    //     ioctl(fd, UI_SET_EVBIT, EV_KEY);
    //     ioctl(fd, UI_SET_KEYBIT, KEY_SPACE);

    //     memset(&usetup, 0, sizeof(usetup));
    //     usetup.id.bustype = BUS_USB;
    //     usetup.id.vendor = 0x1234;  /* sample vendor */
    //     usetup.id.product = 0x5678; /* sample product */
    //     strcpy(usetup.name, "Example device");

    //     ioctl(fd, UI_DEV_SETUP, &usetup);
    //     ioctl(fd, UI_DEV_CREATE);

    //     /*
    // * On UI_DEV_CREATE the kernel will create the device node for this
    // * device. We are inserting a pause here so that userspace has time
    // * to detect, initialize the new device, and can start listening to
    // * the event, otherwise it will not notice the event we are about
    // * to send. This pause is only needed in our example code!
    // */
    //     sleep(1);

    //     /* Key press, report the event, send key release, and report again */
    //     // printf("now looping d key!\n");
    //     // while (true)
    //     // {
    //         emit(fd, EV_KEY, KEY_D, 1);
    //         emit(fd, EV_SYN, SYN_REPORT, 0);
    //         emit(fd, EV_KEY, KEY_D, 0);
    //         emit(fd, EV_SYN, SYN_REPORT, 0);
    //     //}

    //     /*
    // * Give userspace some time to read the events before we destroy the
    // * device with UI_DEV_DESTOY.
    // */
    //     sleep(1);

    //     ioctl(fd, UI_DEV_DESTROY);
    //     close(fd);

    //     return 0;
    // }

    void print_sticks(const uchar &data0, const uchar &data1, const uchar &data2, const uchar &data3, const uchar &data4, const uchar &data5)
    {
        uchar left_x = ((data1 & 0x0F) << 4) | ((data0 & 0xF0) >> 4);
        uchar left_y = data2;
        uchar right_x = ((data4 & 0x0F) << 4) | ((data3 & 0xF0) >> 4);
        uchar right_y = data5;

        map_sticks(left_x, left_y, right_x, right_y);

        clear_console();
        yellow();
        printf("left_x %d\n", left_x);
        printf("left_y %d\n", left_y);
        printf("right_x %d\n", right_x);
        printf("right_y %d\n\n", right_y);
        normal();

        // if(left_x == 0x00 || left_y == 0x00 || right_x == 0x00 || right_y == 0x00 ) {
        //     return -1;
        // }
        // return 0;
    }

    void print_buttons(const uchar &left, const uchar &mid, const uchar &right)
    {
        //uchar left = buttons[0];
        //uchar mid = buttons[1];
        //uchar right = buttons[2];

        if (left & byte_button_value(d_left))
            printf("d_left\n");
        if (left & byte_button_value(d_right))
            printf("d_right\n");
        if (left & byte_button_value(d_up))
            printf("d_up\n");
        if (left & byte_button_value(d_down))
            printf("d_down\n");
        if (left & byte_button_value(L1))
            printf("L1\n");
        if (left & byte_button_value(L2))
            printf("L2\n");
        if (mid & byte_button_value(L3))
            printf("L3\n");
        if (mid & byte_button_value(R3))
            printf("R3\n");
        if (mid & byte_button_value(share))
            printf("share\n");
        if (mid & byte_button_value(home))
        {
            printf("home\n");
        }
        if (mid & byte_button_value(plus))
            printf("plus\n");
        if (mid & byte_button_value(minus))
            printf("minus\n");
        if (right & byte_button_value(A))
            printf("A\n");
        if (right & byte_button_value(B))
            printf("B\n");
        if (right & byte_button_value(X))
            printf("X\n");
        if (right & byte_button_value(Y))
            printf("Y\n");
        if (right & byte_button_value(R1))
            printf("R1\n");
        if (right & byte_button_value(R2))
            printf("R2\n");
    }

    void clear_console()
    {
        system("clear");
    }

    int poll_input()
    {
        // print_cycle_counter++;
        // if(print_cycle_counter++ > n_print_cycle) {
        //     timer();
        // }
        if (!controller_ptr)
        {
            printf("%sERROR: Controller pointer is nullptr%s\n", KRED, KNRM);
            return -1;
        }

        auto dat = send_command(get_input, empty);

        if (detect_useless_data(dat[0]))
        {
            //printf("detected useless data!\n");
            return 0;
        }

        send_subcommand(0x1, ledCommand, led_calibrated); //XXX way too often

        if (dat[0x0e] & byte_button_value(home))
        {
            decalibrate();
        }
        //print_buttons(dat[0x0f], dat[0x0e], dat[0x0d]);
        //print_sticks(dat[0x10], dat[0x11], dat[0x12], dat[0x13], dat[0x14], dat[0x15]);
        //print_exchange_array(dat);
        return 0;
    }

    void calibrate()
    {
        if (read_calibration_from_file)
        {
            std::ifstream myReadFile;
            uchar output[8];
            myReadFile.open("procon_calibration_data", std::ios::in | std::ios::binary);
            if (myReadFile)
            {

                //while (!myReadFile.eof())

                myReadFile.read((char *)&left_x_min, sizeof(uchar));
                myReadFile.read((char *)&left_x_max, sizeof(uchar));
                myReadFile.read((char *)&left_y_min, sizeof(uchar));
                myReadFile.read((char *)&left_y_max, sizeof(uchar));
                myReadFile.read((char *)&right_x_min, sizeof(uchar));
                myReadFile.read((char *)&right_x_max, sizeof(uchar));
                myReadFile.read((char *)&right_y_min, sizeof(uchar));
                myReadFile.read((char *)&right_y_max, sizeof(uchar));

                green();
                printf("Read calibration data from file! ");
                yellow();
                printf("Use --calibrate or -c to calibrate again!\n");
                normal();

                calibrated = true;
                send_subcommand(0x1, ledCommand, led_calibrated);

                return;

            }

            myReadFile.close();
        }

        if (!controller_ptr)
        {
            printf("%sERROR: Controller pointer is nullptr%s\n", KRED, KNRM);
            return;
        }

        auto dat = send_command(get_input, empty);

        if (detect_useless_data(dat[0]))
        {
            //printf("detected useless data!\n");
            return;
        }

        //print_buttons(dat[0x0f], dat[0x0e], dat[0x0d]);
        //print_sticks(dat[0x10], dat[0x11], dat[0x12], dat[0x13], dat[0x14], dat[0x15]);
        //print_exchange_array(dat);

        send_subcommand(0x1, ledCommand, led_calibration); //XXX way too often

        bool cal = do_calibrate(dat[0x10], dat[0x11], dat[0x12], dat[0x13], dat[0x14], dat[0x15], dat[0x0e]);
        if (cal)
        {
            calibrated = true;
            send_subcommand(0x1, ledCommand, led_calibrated);
            //printf("finished calibration\n");
            //usleep(1000000);

            //write calibration data to file
            std::ofstream calibration_file;
            calibration_file.open("procon_calibration_data", std::ios::out | std::ios::binary);
            calibration_file.write((char *)&left_x_min, sizeof(uchar));
            calibration_file.write((char *)&left_x_max, sizeof(uchar));
            calibration_file.write((char *)&left_y_min, sizeof(uchar));
            calibration_file.write((char *)&left_y_max, sizeof(uchar));
            calibration_file.write((char *)&right_x_min, sizeof(uchar));
            calibration_file.write((char *)&right_x_max, sizeof(uchar));
            calibration_file.write((char *)&right_y_min, sizeof(uchar));
            calibration_file.write((char *)&right_y_max, sizeof(uchar));
            calibration_file.close();
            green();
            printf("Wrote calibration data to file!\n");
            normal();
        }

        // std::ofstream out("calibration_data");
        // if (!out)
        // {
        //     return;
        // }

        // printf("wrote text\n");

        // out.close();
    }

    bool do_calibrate(const uchar &stick0, const uchar &stick1, const uchar &stick2, const uchar &stick3, const uchar &stick4, const uchar &stick5, const uchar &mid_buttons)
    {
        uchar left_x = ((stick1 & 0x0F) << 4) | ((stick0 & 0xF0) >> 4);
        uchar left_y = stick2;
        uchar right_x = ((stick4 & 0x0F) << 4) | ((stick3 & 0xF0) >> 4);
        uchar right_y = stick5;

        left_x_min = (left_x < left_x_min) ? left_x : left_x_min;
        left_y_min = (left_y < left_y_min) ? left_y : left_y_min;
        right_x_min = (right_x < right_x_min) ? right_x : right_x_min;
        right_y_min = (right_y < right_y_min) ? right_y : right_y_min;
        left_x_max = (left_x > left_x_max) ? left_x : left_x_max;
        left_y_max = (left_y > left_y_max) ? left_y : left_y_max;
        right_x_max = (right_x > right_x_max) ? right_x : right_x_max;
        right_y_max = (right_y > right_y_max) ? right_y : right_y_max;

        // clear_console();
        // printf("left_x_min: %u\n", left_x_min);
        // printf("left_y_min: %u\n", left_y_min);
        // printf("right_x_min: %u\n", right_x_min);
        // printf("right_y_min: %u\n", right_y_min);
        // printf("left_x_max: %u\n", left_x_max);
        // printf("left_y_max: %u\n", left_y_max);
        // printf("right_x_max: %u\n", right_x_max);
        // printf("right_y_max: %u\n\n", right_y_max);


        return (mid_buttons & byte_button_value(share));
    }

    void decalibrate()
    {
        left_x_min = center;
        left_x_max = center;
        left_y_min = center;
        left_x_max = center;
        right_x_min = center;
        right_x_max = center;
        right_y_min = center;
        right_x_max = center;
        calibrated = false;
        send_subcommand(0x1, ledCommand, led_calibration);
        red();
        printf("Controller decalibrated!\n");
        yellow();
        printf("Perform calibration again and press the square share button!\n");
        normal();
        read_calibration_from_file = false;
        //usleep(1000*1000);
    }

    const void map_sticks(uchar &left_x, uchar &left_y, uchar &right_x, uchar &right_y)
    {
        left_x = uchar(clamp((float)(left_x - left_x_min) / (float)(left_x_max - left_x_min) * 255.f));
        left_y = uchar(clamp((float)(left_y - left_y_min) / (float)(left_y_max - left_y_min) * 255.f));
        right_x = uchar(clamp((float)(right_x - right_x_min) / (float)(right_x_max - right_x_min) * 255.f));
        right_y = uchar(clamp((float)(right_y - right_y_min) / (float)(right_y_max - right_y_min) * 255.f));
    }

    static const float clamp(float inp)
    {
        if (inp < 0.5f)
            return 0.5f;
        if (inp > 254.5f)
        {
            return 254.5f;
        }
        return inp;
    }

    int try_read_bad_data()
    {

        if (!controller_ptr)
        {
            printf("%sERROR: Controller pointer is nullptr%s\n", KRED, KNRM);
            return -1;
        }

        auto dat = send_command(get_input, empty);

        if (detect_useless_data(dat[0]))
        {
            return 0;
        }

        if (detect_bad_data(dat[0], dat[1]))
        {
            //print_exchange_array(dat);
            return -1;
        }

        return 0;
    }

    /* Hackishly detects when the controller is trapped in a bad loop. 
    Nothing to do here, need to restart driver :(*/
    bool detect_bad_data(const uchar &dat1, const uchar &dat2)
    {
        return (dat2 == 0x01 && dat1 == 0x81) ? true : bad_data_detected;
    }

    bool detect_useless_data(const uchar &dat)
    {
        if (dat == 0x30)
            n_bad_data_thirty++;
        if (dat == 0x00)
            n_bad_data_zero++;
        return (dat == 0x30 || dat == 0x00);
    }

    void print_exchange_array(exchange_array arr)
    {
        bool redcol = false;
        if (arr[0] != 0x30)
            yellow();
        else
        {
            red();
            redcol = true;
        }
        for (size_t i = 0; i < 20; ++i)
        {
            if (arr[i] == 0x00)
            {
                blue();
            }
            else
            {
                if (redcol)
                {
                    red();
                }
                else
                {
                    yellow();
                }
            }
            printf("%02X ", arr[i]);
        }
        normal();
        printf("\n");
        fflush(stdout);
    }

    void fill_new_array(exchange_array arr)
    {
        for (size_t i = 0; i < 20; ++i)
        {
            if (arr[i] == 0x00)
                continue;

            if (first[i] == 0x00)
            {
                first[i] = arr[i];
                continue;
            }

            if (first[i] == arr[i])
            {
                continue;
            }

            if (second[i] == 0x00)
            {
                second[i] = arr[i];
                continue;
            }

            if (second[i] == arr[i])
            {
                continue;
            }

            if (third[i] == 0x00)
            {
                third[i] = arr[i];
            }

            if (third[i] == arr[i])
            {
                continue;
            }

            if (fourth[i] == 0x00)
            {
                fourth[i] = arr[i];
            }

            if (fourth[i] == arr[i])
            {
                continue;
            }

            if (fifth[i] == 0x00)
            {
                fifth[i] = arr[i];
            }

            if (fifth[i] == arr[i])
            {
                continue;
            }

            if (sixth[i] == 0x00)
            {
                sixth[i] = arr[i];
            }
        }
    }

    void print_new_array(exchange_array arr)
    {

        red();
        for (size_t i = 0; i < 20; ++i)
        {
            printf("%02X ", i);
        }
        printf("\n");

        for (size_t i = 0; i < 20; ++i)
        {
            if (first[i] == 0x00)
                blue();
            else
                yellow();
            printf("%02X ", first[i]);
        }
        printf("\n");
        for (size_t i = 0; i < 20; ++i)
        {
            if (second[i] == 0x00)
                blue();
            else
                yellow();
            printf("%02X ", second[i]);
        }
        printf("\n");
        for (size_t i = 0; i < 20; ++i)
        {
            if (third[i] == 0x00)
                blue();
            else
                yellow();
            printf("%02X ", third[i]);
        }
        printf("\n");
        for (size_t i = 0; i < 20; ++i)
        {
            if (fourth[i] == 0x00)
                blue();
            else
                yellow();
            printf("%02X ", fourth[i]);
        }
        printf("\n");
        for (size_t i = 0; i < 20; ++i)
        {
            if (fifth[i] == 0x00)
                blue();
            else
                yellow();
            printf("%02X ", fifth[i]);
        }
        printf("\n");
        for (size_t i = 0; i < 20; ++i)
        {
            if (sixth[i] == 0x00)
                blue();
            else
                yellow();
            printf("%02X ", sixth[i]);
        }
        printf("\n\n");
    }

    int read(hid_device *device, unsigned char *data, size_t size)
    {
        int ret = hid_read(device, data, size);
        if (ret < 0)
        {
            printf("%sERROR: Couldn't read from device nr. %u%s\n", KRED, n_controller, KNRM);
        }
        return ret;
    }

    int open_device(unsigned short vendor_id, unsigned short product_id, const wchar_t *serial_number, unsigned short n_controll)
    {
        controller_ptr = hid_open(vendor_id, product_id, serial_number);
        is_opened = true;

        if (!controller_ptr)
        {
            return -1;
        }

        n_controller = n_controll;
        ven_id = vendor_id;
        prod_id = product_id;

        // if (false)
        // { //!exchange(handshake)) { //need std::optional
        //     red();
        //     printf("ERROR: exchange handshake failed!\n");
        //     normal();
        // }

        //set_non_blocking();

        exchange(switch_baudrate);
        exchange(handshake);

        //the next part will sometimes fail, then need to reopen device via hidapi
        int read_failed;
        exchange(hid_only_mode, true, &read_failed);
        if (read_failed < 0)
        {
            return -2;
        }

        send_subcommand(0x1, ledCommand, led_calibration);

        usleep(100 * 1000);

        return 0;
    }

    void set_non_blocking()
    {
        if (hid_set_nonblocking(controller_ptr, 1) < 0)
        {
            printf("%sERROR: Couldn't set controller %u to non-blocking%s\n", KRED, n_controller, KNRM);
        }
    }

    void set_blocking()
    {
        if (hid_set_nonblocking(controller_ptr, 0) < 0)
        {
            printf("%sERROR: Couldn't set controller %u to blocking%s\n", KRED, n_controller, KNRM);
        }
    }

    void close_device()
    {
        if (!is_opened)
            return;
        is_opened = false;
        if (controller_ptr)
        {
            hid_close(controller_ptr);
            blue();
            //printf("Closed controller nr. %u\n", n_controller);
            normal();
        }
    }

    static const void red()
    {
        printf("%s", KRED);
    }
    static const void normal()
    {
        printf("%s", KNRM);
    }
    static const void blue()
    {
        printf("%s", KBLU);
    }
    static const void yellow()
    {
        printf("%s", KYEL);
    }
    static const void green()
    {
        printf("%s", KGRN);
    }
    std::clock_t last_time;

    std::array<uchar, 20> first{{0x0}};
    std::array<uchar, 20> second{{0x0}};
    std::array<uchar, 20> third{{0x0}};
    std::array<uchar, 20> fourth{{0x0}};
    std::array<uchar, 20> fifth{{0x0}};
    std::array<uchar, 20> sixth{{0x0}};

    uchar rumble_counter{0};
    const std::array<uchar, 1> led_calibration{{0xff}};
    const std::array<uchar, 1> led_calibrated{{0x01}};
    const std::array<uchar, 0> empty{{}};
    const std::array<uchar, 2> handshake{{0x80, 0x02}};
    const std::array<uchar, 2> switch_baudrate{{0x80, 0x03}};
    const std::array<uchar, 2> hid_only_mode{{0x80, 0x04}};

    bool bad_data_detected = false;

    hid_device *controller_ptr;

    unsigned short ven_id;
    unsigned short prod_id;
    unsigned short n_controller;

    uint n_print_cycle = 1000;
    uint print_cycle_counter = 0;
    uint n_bad_data_zero = 0;
    uint n_bad_data_thirty = 0;

    bool is_opened = false;
    bool calibrated = false;
    bool read_calibration_from_file = true; // will be set to false in decalibrate or with flags

    uchar left_x_min = 0x7e;
    uchar left_y_min = 0x7e;
    uchar right_x_min = 0x7e;
    uchar right_y_min = 0x7e;
    uchar left_x_max = 0x7e;
    uchar left_y_max = 0x7e;
    uchar right_x_max = 0x7e;
    uchar right_y_max = 0x7e;
};
