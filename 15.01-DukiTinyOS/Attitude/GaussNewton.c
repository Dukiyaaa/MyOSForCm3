//ps:本篇代码完全来自于wjy
#include "GaussNewton.h"

// 全局变量定义
double V[M][3];   // 实际输出矩阵
double K[3][3];   // 比例误差矩阵
double A[M][3];   // 理想输出矩阵
double B[3];      // 零漂误差矩阵
double P[6];      // 参数矩阵
double e[M];      // 误差矩阵

/**
 * @brief 打印矩阵
 * @param matrix 待打印的矩阵
 * @param n 矩阵大小（n x n）
 */
void printMatrix(double matrix[M][M], int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%lf ", matrix[i][j]);
        }
        printf("\n");
    }
}

/**
 * @brief 计算残差
 * @param V 实际输出矩阵
 * @param e 残差矩阵
 * @param P 参数矩阵
 */
void computeResiduals(double V[M][3], double e[M], double P[6]) {
    for (int i = 0; i < M; i++) {
        e[i] = (V[i][0] - P[3]) * (V[i][0] - P[3]) * P[0] * P[0] +
               (V[i][1] - P[4]) * (V[i][1] - P[4]) * P[1] * P[1] +
               (V[i][2] - P[5]) * (V[i][2] - P[5]) * P[2] * P[2] - 1.0f;
    }
}

/**
 * @brief 计算雅可比矩阵
 * @param A 理想输出矩阵
 * @param J 雅可比矩阵
 * @param P 参数矩阵
 */
void computeJacobian(double A[M][3], double J[M][M], double P[M]) {
    for (int i = 0; i < M; i++) {
        J[i][0] = 2 * P[0] * (V[i][0] - P[3]) * (V[i][0] - P[3]);
        J[i][1] = 2 * P[1] * (V[i][1] - P[4]) * (V[i][1] - P[4]);
        J[i][2] = 2 * P[2] * (V[i][2] - P[5]) * (V[i][2] - P[5]);
        J[i][3] = -2 * P[0] * P[0] * (V[i][0] - P[3]);
        J[i][4] = -2 * P[1] * P[1] * (V[i][1] - P[4]);
        J[i][5] = -2 * P[2] * P[2] * (V[i][2] - P[5]);
    }
}

/**
 * @brief 计算矩阵的逆矩阵
 * @param matrix 原始矩阵
 * @param inverse 存储逆矩阵
 * @param n 矩阵大小
 * @return 1: 成功, 0: 失败（不可逆）
 */
int inverseMatrix(double matrix[M][M], double inverse[M][M], int n) {
    double augmented[M][2 * M];
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            augmented[i][j] = matrix[i][j];
        }
        for (int j = 0; j < n; j++) {
            augmented[i][j + n] = (i == j) ? 1 : 0;  // 单位矩阵
        }
    }

    for (int i = 0; i < n; i++) {
        double MEl = fabs(augmented[i][i]);
        int MRow = i;
        for (int k = i + 1; k < n; k++) {
            if (fabs(augmented[k][i]) > MEl) {
                MEl = fabs(augmented[k][i]);
                MRow = k;
            }
        }

        for (int k = i; k < 2 * n; k++) {
            double tmp = augmented[MRow][k];
            augmented[MRow][k] = augmented[i][k];
            augmented[i][k] = tmp;
        }

        double divisor = augmented[i][i];
        if (divisor == 0) {
            return 0;
        }
        for (int k = 0; k < 2 * n; k++) {
            augmented[i][k] /= divisor;
        }

        for (int k = 0; k < n; k++) {
            if (k != i) {
                double factor = augmented[k][i];
                for (int j = 0; j < 2 * n; j++) {
                    augmented[k][j] -= factor * augmented[i][j];
                }
            }
        }
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            inverse[i][j] = augmented[i][j + n];
        }
    }

    return 1;
}

/**
 * @brief 高斯牛顿迭代校准传感器
 */
void gaussNewtonCalibration() {
    double J[M][M];
    double J_T[M][M];
    double J_T_Mul_J[M][M];
    double J_T_Mul_J_N[M][M];
    double J_T_Mul_e[M];
    double unname[M];
    double delt = 0;

    while (1) {
        delt = 0;
        memset(J, 0, sizeof(J));
        memset(J_T, 0, sizeof(J_T));
        memset(J_T_Mul_J, 0, sizeof(J_T_Mul_J));
        memset(J_T_Mul_J_N, 0, sizeof(J_T_Mul_J_N));
        memset(J_T_Mul_e, 0, sizeof(J_T_Mul_e));
        memset(unname, 0, sizeof(unname));

        computeResiduals(V, e, P);
        computeJacobian(A, J, P);

        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                J_T[j][i] = J[i][j];
            }
        }

        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                for (int k = 0; k < M; k++) {
                    J_T_Mul_J[i][j] += J_T[i][k] * J[k][j];
                }
            }
        }

        if (!inverseMatrix(J_T_Mul_J, J_T_Mul_J_N, M)) {
            printf("矩阵不可逆\n");
            break;
        }

        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                J_T_Mul_e[i] += J_T[i][j] * e[j];
            }
        }

        for (int i = 0; i < M; i++) {
            for (int j = 0; j < M; j++) {
                unname[i] += J_T_Mul_J_N[i][j] * J_T_Mul_e[j];
            }
        }

        for (int i = 0; i < M; i++) {
            P[i] -= unname[i] * 0.01;
        }

        for (int i = 0; i < M; i++) {
            delt += unname[i] * unname[i];
        }

        if (delt < ThreshodValue) {
            K[0][0] = P[0];
            K[1][1] = P[1];
            K[2][2] = P[2];
            B[0] = P[3];
            B[1] = P[4];
            B[2] = P[5];
            break;
        }
    }
}

///*单独作为一个程序跑，放在在项目里就把main注释了*/

//int main(){
//    // 写入6组实际值和理想值V和A
//    // 平放

//    V[0][0]=-0.01;
//    V[0][1]=0.00;
//    V[0][2]=1.52;
//    A[0][0]=0;
//    A[0][1]=0;
//    A[0][2]=1;

//    //倒立
//    V[1][0]=0.11;
//    V[1][1]=-0.04;
//    V[1][2]=-0.50;
//    A[1][0]=0;
//    A[1][1]=0;
//    A[1][2]=-1;

//    //左倾
//    V[2][0]=0.09;
//    V[2][1]=-1.00;
//    V[2][2]=0.41;
//    A[2][0]=0;
//    A[2][1]=-1;
//    A[2][2]=0;

//    //右倾
//    V[3][0]=0.02;
//    V[3][1]=0.98;
//    V[3][2]=0.52;
//    A[3][0]=0;
//    A[3][1]=1;
//    A[3][2]=0;

//    //前扑
//    V[4][0]=-0.96;
//    V[4][1]=-0.03;
//    V[4][2]=0.50;
//    A[4][0]=-1;
//    A[4][1]=0;
//    A[4][2]=0;

//    //后仰
//    V[5][0]=1.01;
//    V[5][1]=0.22;
//    V[5][2]=0.51;
//    A[5][0]=1;
//    A[5][1]=0;
//    A[5][2]=0;

//    //手动输入一组P值
//    P[0]=1;
//    P[1]=0.98;
//    P[2]=0.956;
//    P[3]=0.04;
//    P[4]=-0.012;
//    P[5]=-0.075;

//    gaussNewtonCalibration();
//    for(int r=0;r<6;r++){
//        printf("#define\tP%d\t%f\n",r,P[r]);
//    }
//}