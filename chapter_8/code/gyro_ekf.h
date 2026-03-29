#ifndef GYRO_EKF_H
#define GYRO_EKF_H

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        double x[3];       /* [theta, omega, bias] */
        double P[3][3];    /* State covariance */
        double Q[3][3];    /* Process noise covariance */
        double R;          /* Measurement noise variance for omega */
        double dt;         /* Fixed sample interval */
        double gate_sigma; /* Innovation gate; <=0 disables gate */
    } GyroEkf;

    /* Initialize filter with fixed dt and zero initial state. */
    void gyro_ekf_init(GyroEkf *ekf, double dt);

    /* Optional: set process/measurement noise variances. */
    void gyro_ekf_set_noise(GyroEkf *ekf,
                            double q_theta,
                            double q_omega,
                            double q_bias,
                            double r_omega);

    /* Execute one EKF step using gyro measurements. Returns estimated angle. */
    double gyro_ekf_step(GyroEkf *ekf, double omega_meas, double alpha_meas);

    /* Accessor for current angle estimate. */
    double gyro_ekf_get_angle(const GyroEkf *ekf);

#ifdef __cplusplus
}
#endif

#endif