#ifndef __WRISTBAND_TRACKER_H_
#define __WRISTBAND_TRACKER_H_

#include "videoprocessor.h"
#include <string>
#include <sstream>

class Wristband{
public:
    Wristband():find(false),x(0.0), y(0.0), z(0.0), id(0), just(true),
        minOffset(0), maxOffset(100), couter(0){}
    std::vector<cv::Point> startContours;
    double startArea;
    cv::Point startPoint;

    std::vector<cv::Point> currentContours;
    double currentArea;
    cv::Point currentPoint;

    float x, y, z;

    bool find; //fasle: not this wristband. true: have this wristband
    int id; //1 left hand . 2 right hand . 0 left or right hand
    bool just;

    std::vector<cv::Point> bigContours;

    float minOffset, maxOffset;
    long couter;
    void getXYZOffset(){

//        cv::Point2f center_prev, center_new;
//        float radius_prev, radius_new;
//        cv::minEnclosingCircle(cv::Mat(currentContours), center_new, radius_new);
//        cv::minEnclosingCircle(cv::Mat(startContours), center_prev, radius_prev);
//        z = (radius_new - radius_prev);
        couter++;
        if(couter > 5){
            x = (currentPoint.x - startPoint.x);
            y = -(currentPoint.y - startPoint.y);

            z = (currentArea - startArea)/20.0;
        } else{
            startArea = currentArea;
            startPoint = currentPoint;
            startContours = currentContours;
        }
    }

};

class WristbandTracker: public FrameProcessor{
public:
    WristbandTracker(): wristbandNumber(0){}

    std::vector<Wristband> wristband;
    int wristbandNumber;

    float move_x, move_y, move_z;
    float rotation_x, rotation_y, rotation_z;
    float scale;

    void process(cv::Mat& img, cv::Mat& out) {

        cv::Mat skinMask;
        HSVBin(img, skinMask);

        std::vector<std::vector<cv::Point> > contours;
        std::vector<cv::Vec4i> hierarchy;
//        getContours(img, skinMask, contours);
        getContours(img, skinMask, contours, hierarchy);
        std::vector<cv::Point> contours_center;
        calcHullCenter(contours, contours_center);

//        detectionWristband(contours, contours_center);
        detectionWristband(contours, contours_center, hierarchy);
        out = img.clone();

        std::vector<Wristband>::iterator it = wristband.begin();
        std::vector<std::vector<cv::Point> > contoursPrint;
        while(it != wristband.end()){
            if((*it).find == false);
            else
                contoursPrint.push_back((*it).currentContours);
            it++;
        }
        cv::drawContours(out, contours, -1, cv::Scalar(0, 0, 255), 2);
        cv::drawContours(out, contoursPrint, -1, cv::Scalar(0, 255, 0), 2);

//        drawHullIndex(out, contours_center);
        drawHullIndex(out);

        drawTrackDistance(out);
        drawAxis(out);

        getControlers();
        drawControlers(out);
    }
private:
    void showImage(const string &name, const cv::Mat image){
        cv::namedWindow(name);
        cv::imshow(name, image);
    }

    void HSVBin(const cv::Mat &img, cv::Mat &mask){
        cv::Mat hsv;
        cv::cvtColor(img, hsv, CV_RGB2HSV);
        cv::inRange(hsv, cv::Scalar(100, 50, 0), cv::Scalar(125, 255, 255), mask);

    //    cv::Mat res;
    //    cv::bitwise_and(img, img, res, mask);
    //    showImage("Before morphologyEx", res);
    }

    void getContours(const cv::Mat img, const cv::Mat &mask, std::vector<std::vector<cv::Point> > &validContours){
        cv::Mat kernel(5, 5, CV_8U, cv::Scalar(1));
        cv::Mat closed;
        cv::morphologyEx(mask, closed, cv::MORPH_OPEN, kernel);
        cv::morphologyEx(closed, closed, cv::MORPH_CLOSE, kernel);

    //    cv::Mat res;
    //    cv::bitwise_and(img, img, res, mask);
    //    showImage("After morphologyEx", res);

        std::vector<std::vector<cv::Point> > contours;
        cv::findContours(closed, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
        std::vector<std::vector<cv::Point> >::iterator itc = contours.begin();
        while(itc != contours.end()){
            if(cv::contourArea(*itc) > 1200){
                std::vector<cv::Point> hull;
                cv::convexHull(cv::Mat(*itc), hull);
                validContours.push_back(hull);
            }
            itc++;
        }
    }

    void getContours(const cv::Mat img, const cv::Mat &mask,
                     std::vector<std::vector<cv::Point> > &validContours, std::vector<cv::Vec4i> &validHierarchy){
        cv::Mat kernel(5, 5, CV_8U, cv::Scalar(1));
        cv::Mat closed;
        cv::morphologyEx(mask, closed, cv::MORPH_OPEN, kernel);
        cv::morphologyEx(closed, closed, cv::MORPH_CLOSE, kernel);

    //    cv::Mat res;
    //    cv::bitwise_and(img, img, res, mask);
    //    showImage("After morphologyEx", res);

        std::vector<std::vector<cv::Point> > contours;
        std::vector<cv::Vec4i> hierarchy;
        cv::findContours(closed, contours, hierarchy, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);
//        cv::drawContours(img, contours, -1, cv::Scalar(255, 0, 0), 2);

        std::vector<std::vector<cv::Point> >::iterator itc = contours.begin();
        std::vector<cv::Vec4i>::iterator ith = hierarchy.begin();
        int wrist_num = 0;
        while(itc != contours.end()){
            if(cv::contourArea(*itc) > 1000){// filter little hull
                std::vector<cv::Point> hull;
                cv::convexHull(cv::Mat(*itc), hull);
                validContours.push_back(hull);
//                validContours.push_back(*itc);
                cv::Vec4i hie;
                hie[0] = -1;

                if(((*ith)[3]==-1) && ((*ith)[2]!=-1)){
                    std::vector<std::vector<cv::Point> >::iterator itc_temp = ((contours.begin())+(*ith)[2]);
                    std::vector<cv::Vec4i>::iterator ith_temp = hierarchy.begin() + (*ith)[2];
                    bool stop = true;
                    while(stop){
                        if(cv::contourArea(*itc_temp) > 1000){
                            hie[0] = 1;
                            wrist_num++;
                            stop = false;
                        }
                        if((*ith_temp)[1] == -1)
                            stop = false;
                        else{
                            itc_temp = (contours.begin()+(*ith_temp)[1]);
                            ith_temp = hierarchy.begin() + (*ith_temp)[1];
                        }
                    }
                }
                validHierarchy.push_back(hie);
            }
            itc++;
            ith++;
        }
        (this->wristbandNumber) = wrist_num;
        //std::cout << "wristbandNumber is " << wristbandNumber << std::endl;
    }

    string itoa(int i){
        std::stringstream s;
        s << i;
        return s.str();
    }

    string ptoa(cv::Point p){
        std::stringstream s;
        s << "(" << p.x << ", " << p.y << ")" ;
        return s.str();
    }

    void calcHullCenter(std::vector<std::vector<cv::Point> > &contours, std::vector<cv::Point> &contours_center){

        std::vector<std::vector<cv::Point> >::iterator itc = contours.begin();
        while(itc != contours.end()){
            cv::Moments mom = cv::moments(cv::Mat(*itc++));
            cv::Point center(mom.m10/mom.m00, mom.m01/mom.m00);
            contours_center.push_back(center);
        }
    }

    void drawHullIndex(cv::Mat &img, std::vector<cv::Point> &contours_center){
        int index(0);
        std::vector<cv::Point>::iterator itc = contours_center.begin();
        while(itc != contours_center.end()){
            cv::Point center((*itc).x, (*itc).y);
            itc++;
            cv::circle(img, center,
                       2, cv::Scalar(0, 0, 255), 2);
            center.x = center.x + 5;
            center.y = center.y + 5;

            cv::putText(img, itoa(index++), center, 1, 1.5, cv::Scalar(0, 0, 255), 2);
        }
    }

    void drawHullIndex(cv::Mat &img){
        std::vector<Wristband>::iterator it = wristband.begin();
//        int i=0;
        while(it != wristband.end()){
            if((*it).find == false)
                ;
            else{
                cv::Point center((*it).currentPoint.x, (*it).currentPoint.y);
                cv::circle(img, center,
                           2, cv::Scalar(0, 0, 255), 2);
                center.x = center.x + 5;
                center.y = center.y + 5;

//                cv::putText(img, itoa((*it).id), center, 1, 1.5, cv::Scalar(0, 0, 255), 2);

                if((*it).id == 1)
                    cv::putText(img, "Left", center, 1, 1.5, cv::Scalar(150, 240, 50), 2);
                else if((*it).id == 2)
                    cv::putText(img, "Right", center, 1, 1.5, cv::Scalar(150, 240, 50), 2);

                center.x = (*it).currentPoint.x - 70;
                center.y = (*it).currentPoint.y - 15;
                cv::putText(img, ptoa((*it).currentPoint), center, 1, 1.0, cv::Scalar(255, 0, 255, 100), 1);

//                center.x = center.x + 5;
//                center.y = center.y + 25;
//                if(i == 0)
//                    cv::putText(img, "A", center, 1, 1.5, cv::Scalar(255, 0, 0), 2);
//                else if(i == 1)
//                    cv::putText(img, "B", center, 1, 1.5, cv::Scalar(255, 0, 0), 2);
//                i++;
            }
            it++;
        }
    }

    void detectionWristband(std::vector<std::vector<cv::Point> > &contours, std::vector<cv::Point> &contours_center){
        int contours_num = contours.size();
    //    std::cout << contours_num << std::endl;
        int hull_pair = contours_num/2;
        int wristband_num = 0;
//        std::cout << "hull_pair = " << hull_pair << std::endl;
        if(hull_pair){
            for(int i=0; i<hull_pair; i++){
                double big_hull = cv::contourArea(contours[i*2]);
                double small_hull = cv::contourArea(contours[i*2+1]);
    //            std::cout << "big " << big_hull << std::endl;
    //            std::cout << "small " << small_hull << std::endl;
                if(big_hull > small_hull){
                    if(cv::pointPolygonTest(contours[i*2], contours_center[i*2+1], false) > 0){
                        int j = 0;
                        for(j=0; j<wristband.size(); j++){
                            if(wristband[j].just == false){
                                if(cv::pointPolygonTest(wristband[j].bigContours, contours_center[i*2+1], false) > 0){
                                    wristband[j].currentPoint = contours_center[i*2+1];
                                    wristband[j].currentContours = contours[i*2+1];
                                    wristband[j].currentArea = small_hull;
//                                    wristband[j].x = abs(wristband[j].startPoint.x - wristband[j].currentPoint.x);
//                                    wristband[j].y = abs(wristband[j].startPoint.y - wristband[j].currentPoint.y);
                                   // wristband[j].z = abs(wristband[j].startPoint.z - wrist.currentPoint.z);
                                    wristband[j].find = true;
                                    wristband[j].id = 0;
                                    wristband[j].just = true;
                                    wristband[j].bigContours = contours[i*2];
//                                    std::cout << "wristband[j].startPoint : " << wristband[j].startPoint << ". J = " << j << std::endl;
                                    break;
                                }
                                else{
                                    wristband[j].find = false;
                                }
                            }
                        }
                        if(j == wristband.size()){
                            Wristband wrist;
                            wrist.startPoint = contours_center[i*2+1];
                            wrist.startContours = contours[i*2+1];
                            wrist.startArea = small_hull;
                            wrist.currentPoint = contours_center[i*2+1];
                            wrist.currentContours = contours[i*2+1];
                            wrist.currentArea = small_hull;
//                            wrist.x = 0;
//                            wrist.y = 0;
//                            wrist.z = 0;
                            wrist.find = true;
                            wrist.id = 0;
                            wrist.just = true;
                            wrist.bigContours = contours[i*2];
                            wristband.push_back(wrist);
//                            std::cout << "New Wrist one " << i << std::endl;
                        }
                        wristband_num++;
                    }
                }
            }
        }
        else{
            std::vector<Wristband>::iterator itc = wristband.begin();
            while(itc != wristband.end()){
                itc = wristband.erase(itc);
            }

        }

        //clear wristband
        std::vector<Wristband>::iterator it = wristband.begin();
        while(it != wristband.end()){
            if((*it).find == false){
                it = wristband.erase(it); //clear
            } else {
                (*it).just = false;
                ++it;
            }
        }
        //panduan is left or right.
//        std::cout << "wristband size is " << wristband.size() << std::endl;

        if(wristband.size() == 2){
//            std::cout << "wristband[0].startPoint : " << wristband[0].startPoint << std::endl;
//            std::cout << "wristband[1].startPoint : " << wristband[1].startPoint << std::endl;
            int x_diff = (wristband[0].startPoint.x - wristband[1].startPoint.x);
//            std::cout << "x_diff = " << x_diff << std::endl;
            if(x_diff > 0){
                wristband[0].id = 2;
                wristband[1].id = 1;
//                std::cout << 21 << std::endl;
            }
            else{
                wristband[0].id = 1;
                wristband[1].id = 2;
//                std::cout << 12 << std::endl;
            }
//            wristband[0].id = 2;
//            wristband[1].id = 1;

        }
//        std::cout << "Detection Wristband number is " << wristband_num << std::endl;
    }

    void detectionWristband(std::vector<std::vector<cv::Point> > &contours, std::vector<cv::Point> &contours_center
                            , std::vector<cv::Vec4i> &validHierarchy){
//        int contours_num = contours.size();
//        std::cout << contours_num << std::endl;
//        std::cout << "hull_pair = " << hull_pair << std::endl;
        if(wristbandNumber){
            for(int j=0; j<wristband.size(); j++){
                wristband[j].find = false;
            }

            int i=0;
            std::vector<cv::Vec4i>::iterator ith = validHierarchy.begin();
            while(ith != validHierarchy.end()){
                if((*ith)[0] == 1){
                    double big_hull = cv::contourArea(contours[i]);
                    double small_hull = cv::contourArea(contours[i+1]);
        //            std::cout << "big " << big_hull << std::endl;
        //            std::cout << "small " << small_hull << std::endl;
                    if(big_hull > small_hull){
                        if(cv::pointPolygonTest(contours[i], contours_center[i+1], false) > 0){
                            int j = 0;
                            for(j=0; j<wristband.size(); j++){
                                if(wristband[j].just == false){
                                    if(cv::pointPolygonTest(wristband[j].bigContours, contours_center[i+1], false) > 0){
                                        wristband[j].currentPoint = contours_center[i+1];
                                        wristband[j].currentContours = contours[i+1];
                                        wristband[j].currentArea = small_hull;
    //                                    wristband[j].x = abs(wristband[j].startPoint.x - wristband[j].currentPoint.x);
    //                                    wristband[j].y = abs(wristband[j].startPoint.y - wristband[j].currentPoint.y);
                                       // wristband[j].z = abs(wristband[j].startPoint.z - wrist.currentPoint.z);
                                        wristband[j].getXYZOffset();
                                        wristband[j].find = true;
                                        wristband[j].id = 0;
                                        wristband[j].just = true;
                                        wristband[j].bigContours = contours[i];
    //                                    std::cout << "wristband[j].startPoint : " << wristband[j].startPoint << ". J = " << j << std::endl;
                                        break;
                                    }
                                    else{
                                        wristband[j].find = false;
                                    }
                                }
                            }
                            if(j == wristband.size()){
                                Wristband wrist;
                                wrist.startPoint = contours_center[i+1];
                                wrist.startContours = contours[i+1];
                                wrist.startArea = small_hull;
                                wrist.currentPoint = contours_center[i+1];
                                wrist.currentContours = contours[i+1];
                                wrist.currentArea = small_hull;
    //                            wrist.x = 0;
    //                            wrist.y = 0;
    //                            wrist.z = 0;
                                wrist.find = true;
                                wrist.id = 0;
                                wrist.just = true;
                                wrist.bigContours = contours[i];
                                wristband.push_back(wrist);
    //                            std::cout << "New Wrist one " << i << std::endl;
                            }
    //                        wristband_num++;
                        }
                    }
                }
                i++;
                ith++;
            }
        }
        else{
            std::vector<Wristband>::iterator itc = wristband.begin();
            while(itc != wristband.end()){
                itc = wristband.erase(itc);
            }
        }

        //clear wristband
        std::vector<Wristband>::iterator it = wristband.begin();
        while(it != wristband.end()){
            if((*it).find == false){
                it = wristband.erase(it); //clear
            } else {
                (*it).just = false;
                ++it;
            }
        }
        //panduan is left or right.
//        std::cout << "wristband size is " << wristband.size() << std::endl;

        if(wristband.size() == 2){
//            std::cout << "wristband[0].startPoint : " << wristband[0].startPoint << std::endl;
//            std::cout << "wristband[1].startPoint : " << wristband[1].startPoint << std::endl;
            int x_diff = (wristband[0].startPoint.x - wristband[1].startPoint.x);
//            std::cout << "x_diff = " << x_diff << std::endl;
            if(x_diff > 0){
                wristband[0].id = 2;
                wristband[1].id = 1;
//                std::cout << 21 << std::endl;
            }
            else{
                wristband[0].id = 1;
                wristband[1].id = 2;
//                std::cout << 12 << std::endl;
            }
//            wristband[0].id = 2;
//            wristband[1].id = 1;

        }
//        std::cout << "Detection Wristband number is " << wristband_num << std::endl;
    }


    void drawTrackDistance(cv::Mat &img){
        std::vector<Wristband>::iterator it = wristband.begin();
        while(it != wristband.end()){
            if((*it).find == false)
                ;
            else{
                cv::line(img, (*it).startPoint, (*it).currentPoint, cv::Scalar(0, 255, 0), 2);
            }
            it++;
        }
    }

    void drawArrows(cv::Mat& frame, const vector<cv::Point2f>& prevPts, const vector<cv::Point2f>& nextPts,
                     const Scalar& line_color)
    {
        int line_thickness = 2;

        for (size_t i = 0; i < prevPts.size(); ++i)
        {
            Point p = prevPts[i];
            Point q = nextPts[i];

            double angle = atan2((double) p.y - q.y, (double) p.x - q.x);

            double hypotenuse = sqrt( (double)(p.y - q.y)*(p.y - q.y) + (double)(p.x - q.x)*(p.x - q.x) );

            if (hypotenuse < 1.0)
                continue;

            // Here we lengthen the arrow by a factor of three.
            q.x = (int) (p.x - 3 * hypotenuse * cos(angle));
            q.y = (int) (p.y - 3 * hypotenuse * sin(angle));

            // Now we draw the main line of the arrow.
            cv::line(frame, p, q, line_color, line_thickness);

            // Now draw the tips of the arrow. I do some scaling so that the
            // tips look proportional to the main line of the arrow.

            p.x = (int) (q.x + 9 * cos(angle + CV_PI / 4));
            p.y = (int) (q.y + 9 * sin(angle + CV_PI / 4));
            line(frame, p, q, line_color, line_thickness);

            p.x = (int) (q.x + 9 * cos(angle - CV_PI / 4));
            p.y = (int) (q.y + 9 * sin(angle - CV_PI / 4));
            line(frame, p, q, line_color, line_thickness);
        }
    }

    void drawArrow(cv::Mat& frame, const cv::Point2f& prev, const cv::Point2f& next,
                     const Scalar& line_color)
    {
        int line_thickness = 2;

        Point p = prev;
        Point q = next;

        double angle = atan2((double) p.y - q.y, (double) p.x - q.x);

        double hypotenuse = sqrt( (double)(p.y - q.y)*(p.y - q.y) + (double)(p.x - q.x)*(p.x - q.x) );

        if (hypotenuse < 1.0)
            return;

        // Here we lengthen the arrow by a factor of three.
        q.x = (int) (p.x - 1.0 * hypotenuse * cos(angle));
        q.y = (int) (p.y - 1.0 * hypotenuse * sin(angle));

        // Now we draw the main line of the arrow.
        cv::line(frame, p, q, line_color, line_thickness);

        // Now draw the tips of the arrow. I do some scaling so that the
        // tips look proportional to the main line of the arrow.

        p.x = (int) (q.x + 9.0 * cos(angle + CV_PI / 4));
        p.y = (int) (q.y + 9.0 * sin(angle + CV_PI / 4));
        line(frame, p, q, line_color, line_thickness);

        p.x = (int) (q.x + 9.0 * cos(angle - CV_PI / 4));
        p.y = (int) (q.y + 9.0 * sin(angle - CV_PI / 4));
        line(frame, p, q, line_color, line_thickness);
    }

    void drawAxis(cv::Mat &img){
        std::vector<Wristband>::iterator it = wristband.begin();
        while(it != wristband.end()){
            if((*it).find == false)
                ;
            else{
                cv::Point startPoint, endPoint;
                startPoint = (*it).currentPoint;
                //draw x axis
                endPoint = (*it).currentPoint;
                endPoint.x += (*it).x;
                drawArrow(img, startPoint, endPoint, cv::Scalar(0, 0, 255));
                endPoint.x -= 28;    endPoint.y += 28;
                cv::putText(img, itoa((*it).x), endPoint, 1, 1.5, cv::Scalar(0, 0, 255), 2);

                //draw y axis
                endPoint = (*it).currentPoint;
                endPoint.y -= (*it).y;
                drawArrow(img, startPoint, endPoint, cv::Scalar(0, 255, 0));
                endPoint.x += 8;    endPoint.y += 8;
                cv::putText(img, itoa((*it).y), endPoint, 1, 1.5, cv::Scalar(0, 255, 0), 2);

                //draw z axis
                endPoint = (*it).currentPoint;
                endPoint.y -= (*it).z;
                endPoint.x += (*it).z;
                drawArrow(img, startPoint, endPoint, cv::Scalar(255, 0, 0));
                endPoint.x += 8;    endPoint.y += 8;
                cv::putText(img, itoa((*it).z), endPoint, 1, 1.5, cv::Scalar(255, 0, 0), 2);

            }
            it++;
        }
    }

    void getMoveControlers(){
        move_x = wristband[0].x;
        move_y = wristband[0].y;
        move_z = wristband[0].z;
    }

    void getRotationAndScaleControlers(){
        float left_x, left_y, left_z;
        float right_x, right_y, right_z;
        if(wristband[0].id == 1){ //0:left 1:right
            left_x = wristband[0].x; left_y = wristband[0].y; left_z = wristband[0].z;
            right_x = wristband[1].x; right_y = wristband[1].y; right_z = wristband[1].z;
        } else{ //1:left 0:right
            left_x = wristband[1].x; left_y = wristband[1].y; left_z = wristband[1].z;
            right_x = wristband[0].x; right_y = wristband[0].y; right_z = wristband[0].z;
        }

        float rotation_temp;
        //y Axies rotation
        if((left_y*right_y) > 0){
            rotation_temp = (left_y + right_y)/2.0;
            this->rotation_x = rotation_temp;
            this->rotation_y = 0;
        } else {
            rotation_temp = (left_y - right_y)/2.0;
            this->rotation_x = 0;
            this->rotation_y = rotation_temp;
        }

        {
            rotation_temp = (left_z - right_z)/2.0;
            this->rotation_z = rotation_temp;
        }

    }

    void clearMoveControlers(){
        move_x = 0;
        move_y = 0;
        move_z = 0;
    }

    void clearRotationControlers(){
        this->rotation_x = 0;
        this->rotation_y = 0;
        this->rotation_z = 0;
    }

    void clearScaleControlers(){

    }

    void getControlers(){
        switch(this->wristbandNumber){
        case 0:
            clearMoveControlers();
            clearRotationControlers();
            clearScaleControlers();
            break;
        case 1:
            getMoveControlers();
            clearRotationControlers();
            clearScaleControlers();
            break;
        case 2:
            getRotationAndScaleControlers();
            clearMoveControlers();
            break;
        default:
            break;
        }
    }


    void drawControlers(cv::Mat &img){
        switch(this->wristbandNumber){
        case 0:
            //std::cout << "No Wristband" << std::endl;
            break;
        case 1:{
            cv::Point point;
            point.y = 30;
            point.x = 30;
            std::stringstream s;
            s << "move x:" << this->move_x;
            s << "move y:" << this->move_y;
            s << "move z:" << this->move_z;

            cv::putText(img, s.str(), point, 1, 1.5, cv::Scalar(255, 255, 0), 2);
        }
            break;
        case 2:{
            cv::Point point;
            point.y = 30;
            point.x = 30;
            std::stringstream s;
            s << "rot x:" << this->rotation_x;
            s << "rot y:" << this->rotation_y;
            s << "rot z:" << this->rotation_z;

            cv::putText(img, s.str(), point, 1, 1.5, cv::Scalar(255, 255, 0), 2);
        }
            break;
        default:
            //std::cout << "Error" << std::endl;
            break;
        }
    }
};

#endif
