#include <fstream>

int main() {
    std::ofstream file("test_fichier_qui_nexiste_pas.txt");
    file << "Hello World";
    file.close();
    return 0;
}
