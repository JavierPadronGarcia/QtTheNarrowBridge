#include <QCoreApplication>
#include <QThread>
#include <QSemaphore>
#include <QRandomGenerator>
#include <iostream>
#include <iomanip>

constexpr int VehicleQuantity = 20;
constexpr int MaxVehiclesInBridge = 3;

QSemaphore bridgeQuantity(MaxVehiclesInBridge);
QSemaphore consoleAvailable(1);

QSemaphore variableBlock(1);

int bridgeDir = 0;

QList<int> carsOnBridge;
QList<int> waitingBuffer;

class Vehicle : public QThread {
public:
    Vehicle(int id, int direction) : id(id), direction(direction) {}

    void run() override {

        // ARRIVES AT THE BRIDGE
        consoleAvailable.acquire();
        showOutput("ARRIVES", id, direction);
        consoleAvailable.release();

        bool showed = false;

        while (true) {
            variableBlock.acquire();
            if (bridgeDir == 0 || bridgeDir == direction) {
                bridgeDir = direction;
                consoleAvailable.acquire();
                if(waitingBuffer.contains(id)) waitingBuffer.removeOne(id);
                showOutput("CONTINUES", id, direction);
                consoleAvailable.release();

                variableBlock.release();
                break;
            }

            if (!showed) {
                consoleAvailable.acquire();
                waitingBuffer.append(id);
                showOutput("WAITS", id, direction);
                consoleAvailable.release();
                showed = true;
            }
            variableBlock.release();
            QThread::sleep(1);
        }

        if (bridgeDir == direction) {
            bridgeQuantity.acquire();

            // CROSSES
            variableBlock.acquire();
            carsOnBridge.append(id);
            consoleAvailable.acquire();
            showOutput("CROSSES", id, direction);
            consoleAvailable.release();
            variableBlock.release();
            QThread::sleep(1);

            // LEAVES THE BRIDGE
            variableBlock.acquire();
            if (carsOnBridge.size() == 1) {
                bridgeDir = 0;
            };
            carsOnBridge.removeOne(id);
            bridgeQuantity.release();
            consoleAvailable.acquire();
            showOutput("LEAVES", id, direction);
            consoleAvailable.release();
            variableBlock.release();
        }
    }

private:
    void showOutput(const QString& event, int id, int direction) {
        std::string dirSymbol;
        if (bridgeDir == 0) {
            dirSymbol = "----";
        } else if (bridgeDir == 1) {
            dirSymbol = "<---";
        } else if (bridgeDir == 2) {
            dirSymbol = "--->";
        }

        std::cout << id << " " << std::left << std::setw(8) << event.toStdString()
                  << " Vdir: " << (direction == 1 ? "<---" : "--->") << " BridgeDir: " << dirSymbol
                  << " On bridge: ";
        showCarsOnBridge();
        std::cout << "\t Waiting: ";
        showWaitingCars();
        std::cout << "\n";
    }

    void showCarsOnBridge() {
        if (carsOnBridge.isEmpty()) {
            std::cout << std::setw(1) << "0 ";
        } else {
            for (int carId : carsOnBridge) {
                std::cout << std::setw(1) << carId << " ";
            }
        }
    }

    void showWaitingCars() {
        if(waitingBuffer.isEmpty()){
            std::cout << std::setw(1) << "0 ";
        } else {
            for (int carId : waitingBuffer) {
                std::cout << std::setw(1) << carId << " ";
            }
        }
    }

    int id;
    int direction;
};

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    QList<Vehicle *> vehicles;

    for (int i = 0; i < VehicleQuantity; ++i) {
        int direction = (QRandomGenerator::global()->bounded(2)) + 1;
        Vehicle *vehicle = new Vehicle(i + 1, direction);
        vehicles.append(vehicle);
        vehicle->start();
    }

    for (int i = 0; i < VehicleQuantity; ++i) {
        vehicles[i]->wait();
    }

    return 0;
}

