#include <stdio.h>
#include <stdlib.h>

#include "gyro_ekf.h"

int main(void) {
    const double dt = 0.01;   /* 100 Hz */
    const int steps = 800;

    GyroEkf ekf;
    gyro_ekf_init(&ekf, dt);

    /* Example parameter set, tune against real sensor noise statistics. */
    gyro_ekf_set_noise(&ekf, 1e-6, 5e-4, 1e-7, 2e-3);

    double true_theta = 0.0;
    double true_omega = 0.0;
    const double true_bias = 0.03; /* rad/s */

    printf("k,time,omega_meas,alpha_meas,theta_est,theta_true\n");

    for (int k = 0; k < steps; ++k) {
        const double t = k * dt;

        /* Piecewise motion profile for demonstration. */
        double alpha_true = 0.0;
        if (t < 2.0) {
            alpha_true = 0.8;
        } else if (t < 4.0) {
            alpha_true = 0.0;
        } else if (t < 6.0) {
            alpha_true = -0.6;
        } else {
            alpha_true = 0.0;
        }

        true_omega += alpha_true * dt;
        true_theta += true_omega * dt;

        /* Build synthetic gyro measurements with simple deterministic ripple. */
        const double omega_noise = ((k % 13) - 6) * 0.0015;
        const double alpha_noise = ((k % 11) - 5) * 0.01;

        const double omega_meas = true_omega + true_bias + omega_noise;
        const double alpha_meas = alpha_true + alpha_noise;

        const double theta_est = gyro_ekf_step(&ekf, omega_meas, alpha_meas);

        if (k % 20 == 0) {
            printf("%d,%.3f,%.6f,%.6f,%.6f,%.6f\n",
                   k,
                   t,
                   omega_meas,
                   alpha_meas,
                   theta_est,
                   true_theta);
        }
    }

    return EXIT_SUCCESS;
}