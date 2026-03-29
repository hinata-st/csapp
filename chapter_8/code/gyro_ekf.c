#include "gyro_ekf.h"

#include <math.h>
#include <stddef.h>

static double clamp_min(double v, double min_v)
{
    return (v < min_v) ? min_v : v;
}

void gyro_ekf_init(GyroEkf *ekf, double dt)
{
    if (ekf == NULL)
    {
        return;
    }

    ekf->dt = dt;
    ekf->x[0] = 0.0;
    ekf->x[1] = 0.0;
    ekf->x[2] = 0.0;

    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            ekf->P[i][j] = 0.0;
            ekf->Q[i][j] = 0.0;
        }
    }

    /* Relatively uncertain start; tune as needed. */
    ekf->P[0][0] = 1.0;
    ekf->P[1][1] = 1.0;
    ekf->P[2][2] = 1.0;

    ekf->Q[0][0] = 1e-6;
    ekf->Q[1][1] = 1e-4;
    ekf->Q[2][2] = 1e-7;
    ekf->R = 1e-3;
    ekf->gate_sigma = 4.0;
}

void gyro_ekf_set_noise(GyroEkf *ekf,
                        double q_theta,
                        double q_omega,
                        double q_bias,
                        double r_omega)
{
    if (ekf == NULL)
    {
        return;
    }

    ekf->Q[0][0] = clamp_min(q_theta, 1e-15);
    ekf->Q[1][1] = clamp_min(q_omega, 1e-15);
    ekf->Q[2][2] = clamp_min(q_bias, 1e-15);
    ekf->R = clamp_min(r_omega, 1e-15);
}

double gyro_ekf_step(GyroEkf *ekf, double omega_meas, double alpha_meas)
{
    if (ekf == NULL)
    {
        return 0.0;
    }

    const double dt = ekf->dt;

    /* Predict state: theta += omega*dt, omega += alpha*dt, bias random walk. */
    const double theta_pred = ekf->x[0] + ekf->x[1] * dt;
    const double omega_pred = ekf->x[1] + alpha_meas * dt;
    const double bias_pred = ekf->x[2];

    /* F = d(f)/d(x) */
    const double F[3][3] = {
        {1.0, dt, 0.0},
        {0.0, 1.0, 0.0},
        {0.0, 0.0, 1.0}};

    /* P_pred = F*P*F' + Q */
    double FP[3][3] = {0};
    double P_pred[3][3] = {0};
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            for (int k = 0; k < 3; ++k)
            {
                FP[i][j] += F[i][k] * ekf->P[k][j];
            }
        }
    }
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            for (int k = 0; k < 3; ++k)
            {
                P_pred[i][j] += FP[i][k] * F[j][k];
            }
            P_pred[i][j] += ekf->Q[i][j];
        }
    }

    /* Measurement model: z = h(x) = omega + bias */
    const double z_pred = omega_pred + bias_pred;
    const double innovation = omega_meas - z_pred;
    const double H[3] = {0.0, 1.0, 1.0};

    /* S = H*P*H' + R, scalar in this case. */
    double S = ekf->R;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            S += H[i] * P_pred[i][j] * H[j];
        }
    }
    S = clamp_min(S, 1e-12);

    int do_update = 1;
    if (ekf->gate_sigma > 0.0)
    {
        const double gate = ekf->gate_sigma * sqrt(S);
        if (fabs(innovation) > gate)
        {
            do_update = 0;
        }
    }

    if (do_update)
    {
        /* K = P*H'/S */
        double K[3] = {0};
        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                K[i] += P_pred[i][j] * H[j];
            }
            K[i] /= S;
        }

        /* State update: x = x + K*y */
        ekf->x[0] = theta_pred + K[0] * innovation;
        ekf->x[1] = omega_pred + K[1] * innovation;
        ekf->x[2] = bias_pred + K[2] * innovation;

        /* Joseph form for numerical stability:
           P = (I-KH)P(I-KH)' + K*R*K' */
        double I_KH[3][3] = {
            {1.0 - K[0] * H[0], -K[0] * H[1], -K[0] * H[2]},
            {-K[1] * H[0], 1.0 - K[1] * H[1], -K[1] * H[2]},
            {-K[2] * H[0], -K[2] * H[1], 1.0 - K[2] * H[2]}};

        double temp[3][3] = {0};
        double P_new[3][3] = {0};
        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                for (int k = 0; k < 3; ++k)
                {
                    temp[i][j] += I_KH[i][k] * P_pred[k][j];
                }
            }
        }
        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                for (int k = 0; k < 3; ++k)
                {
                    P_new[i][j] += temp[i][k] * I_KH[j][k];
                }
                P_new[i][j] += K[i] * ekf->R * K[j];
            }
        }

        /* Keep symmetry and positive diagonals. */
        for (int i = 0; i < 3; ++i)
        {
            for (int j = i; j < 3; ++j)
            {
                const double sym = 0.5 * (P_new[i][j] + P_new[j][i]);
                ekf->P[i][j] = sym;
                ekf->P[j][i] = sym;
            }
            ekf->P[i][i] = clamp_min(ekf->P[i][i], 1e-15);
        }
    }
    else
    {
        ekf->x[0] = theta_pred;
        ekf->x[1] = omega_pred;
        ekf->x[2] = bias_pred;

        for (int i = 0; i < 3; ++i)
        {
            for (int j = 0; j < 3; ++j)
            {
                ekf->P[i][j] = P_pred[i][j];
            }
        }
    }

    return ekf->x[0];
}

double gyro_ekf_get_angle(const GyroEkf *ekf)
{
    if (ekf == NULL)
    {
        return 0.0;
    }
    return ekf->x[0];
}