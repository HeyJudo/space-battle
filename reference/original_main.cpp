#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace std;

int main() {
    // Player and enemy health
    int playerHealth = 100;
    int enemyHealth = 100;

    int choice;
    int damage;
    int heal;
    bool defending;

    // Initialize random number generator
    srand(time(0));

    cout << "================================\n";
    cout << "       SPACE BATTLE GAME\n";
    cout << "================================\n";
    cout << "Defeat the enemy spaceship!\n";

    // Continue until one spaceship loses
    while (playerHealth > 0 && enemyHealth > 0) {

        defending = false;

        // Display current health
        cout << "\n---------- STATUS ----------\n";
        cout << "Your Health: " << playerHealth << endl;
        cout << "Enemy Health: " << enemyHealth << endl;

        // Display player actions
        cout << "\nChoose your action:\n";
        cout << "1. Attack\n";
        cout << "2. Defend\n";
        cout << "3. Heal\n";
        cout << "4. Quit Game\n";
        cout << "Enter choice: ";
        cin >> choice;

        // Perform player's action
        switch (choice) {
            case 1:
                // Random damage from 10 to 25
                damage = rand() % 16 + 10;
                enemyHealth -= damage;

                if (enemyHealth < 0)
                    enemyHealth = 0;

                cout << "\nYou attacked the enemy!\n";
                cout << "Damage dealt: " << damage << endl;
                break;

            case 2:
                // Reduce the next enemy attack
                defending = true;
                cout << "\nYou activated your shields!\n";
                break;

            case 3:
                // Restore 15 to 30 health
                heal = rand() % 16 + 15;
                playerHealth += heal;

                if (playerHealth > 100)
                    playerHealth = 100;

                cout << "\nYou repaired your spaceship!\n";
                cout << "Health restored: " << heal << endl;
                break;

            case 4:
                cout << "\nYou left the battle.\n";
                return 0;

            default:
                cout << "\nInvalid choice! Try again.\n";
                continue;
        }

        // Stop if the enemy has been defeated
        if (enemyHealth <= 0)
            break;

        // Enemy chooses a random action
        int enemyAction = rand() % 3 + 1;

        if (enemyAction == 1) {
            // Enemy attacks
            damage = rand() % 16 + 10;

            // Defending reduces incoming damage
            if (defending) {
                damage /= 2;
                cout << "\nYour shields reduced the damage!\n";
            }

            playerHealth -= damage;

            if (playerHealth < 0)
                playerHealth = 0;

            cout << "Enemy attacked your spaceship!\n";
            cout << "Damage received: " << damage << endl;

        } else if (enemyAction == 2) {
            // Enemy defends
            cout << "\nEnemy activated its shields!\n";

        } else {
            // Enemy repairs its spaceship
            heal = rand() % 16 + 15;
            enemyHealth += heal;

            if (enemyHealth > 100)
                enemyHealth = 100;

            cout << "\nEnemy repaired its spaceship!\n";
            cout << "Enemy health restored: " << heal << endl;
        }
    }

    // Display the final result
    cout << "\n================================\n";
    cout << "           GAME OVER\n";
    cout << "================================\n";

    cout << "Your Final Health: " << playerHealth << endl;
    cout << "Enemy Final Health: " << enemyHealth << endl;

    if (playerHealth <= 0)
        cout << "You lost! Your spaceship was destroyed.\n";
    else if (enemyHealth <= 0)
        cout << "Congratulations! You defeated the enemy!\n";

    cout << "\nThanks for playing Space Battle!\n";

    return 0;
}
