#include "ros/ros.h" 
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // Request a service and pass the velocities to it to drive the robot
    ROS_INFO_STREAM("Driving The Robot ...");

    // Request Velocities
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    // Call the drive_bot service and pass the vel commands
    if (!client.call(srv)) {
        ROS_ERROR("Failed to call service command_robot!");
    }
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;

    float white_step = -1.;

    // Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera

    // Jump every 3 steps instead of 1 so that it will always on the same pixel when detecting a ball or not.
    for (int i = 0; i + 2 < img.height * img.step; i = i + 3) {
        // img.data[i], img.data[i+1], and img.data[i+2] corresponds to R, G, and B channel of a pixel
        if (img.data[i] == white_pixel && img.data[i+1] == white_pixel && img.data[i+2] == white_pixel) {
            white_step = i % img.step;
            break;
        }
    }

    // initialize lin_x and ang_z to be zero
    float lin_x = 0, ang_z = 0;

    // This condition here provides more smoothness in movement of the bot
    // althouth it still is kind of juggy.

    // if 0 < white_step <= 0.3 * img.step, the ball is on the left
    if ((white_step <= 0.3 * img.step) && (white_step > 0.)) {
        ang_z = 0.5;
        lin_x = 0.25;
    }
    
    // if 0.3 * img.step < white_step <= 0.4 * img.step, the ball still on the left, but increase lin_x, decrease ang_z
    if ((white_step <= 0.4 * img.step) && (white_step > 0.3)) {
        ang_z = 0.45;
        lin_x = 0.35;
    }
    
    // if 0.6 * img.step <= white_step < 0.7, the ball is on the right 
    else if ((white_step >= 0.6 * img.step) && (white_step < 0.7 * img.step)) {
        ang_z = -0.45;
        lin_x = 0.35;
    }

    // if 0.7 * img.step <= white_step, the ball is still on the right, but decrease lin_x , increase ang_z 
    else if (white_step >= 0.7 * img.step) {
        ang_z = -0.5;
        lin_x = 0.25;
    }

    else if (white_step != -1.) {
        lin_x = 0.5;
    }

    drive_robot(lin_x, ang_z);

}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}
