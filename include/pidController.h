#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

/**
 * @brief A PID controller class, the constructor takes the following parameters:
 * 
 * @param p the P parameter
 * @param i the I parameter
 * @param d the D parameter
 * @param setPoint the target point
 * 
 * When a object has been created the pid() function can be used to calculate a next pid value,
 * this function has two parameters:
 * 
 * @param input the input value
 * @param timeStep the amount of time between this and the last step
 */
class PidController {
    private:
        float p = 0;
        float i = 0;
        float d = 0;

        float prevError = 0;
        float integral = 0;
        float setPoint = 0;

    public:
        PidController(float p, float i, float d, float setPoint);

        ~PidController();

        float pid(float input, float timeStep);
};

#endif