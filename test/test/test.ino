
int readCapteur(int capteur_id) {
    return rand() % 100 + capteur_id*20;
}

void calibration_suivi_lignes(int seuils[5]) {
    int mesures[5][100]; //100 mesures pour chaque capteur

    for (int i = 0; i < 100; i++) {
        for (int c = 0; c < 5; c++) {
            mesures[c][i] = readCapteur(c);
        }
    }

    for (int c = 0; c < 5; c++) {
        int min = mesures[c][0];
        int max = mesures[c][0];
        for (int i = 1; i < 100; i++) {
            if (mesures[c][i] < min)
                min = mesures[c][i];
            if (mesures[c][i] > max)
                max = mesures[c][i];
        }
        seuils[c] = (max + min) / 2;
        printf("Capteur %d : min=%d, max=%d, seuil=%d\n", c, min, max, seuils[c]);
    }
}

void setup(){

}

int main() {

    int seuils[5];
    calibration_suivi_lignes(seuils);

    printf("Seuils moyens : ");
    for (int c = 0; c < 5; ++c)
        printf("%d ", seuils[c]);
    printf("\n");
    return 0;
}