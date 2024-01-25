#include "vehicle.h"
#include <iostream>

Vehicle::Vehicle(QObject *parent)
    : QThread{parent}
{}


Vehicle::Vehicle(const int VehicleQuantity,
                 QSemaphore *vehiclesInBridge,
                 QSemaphore *consoleAvailable,
                 int direction,
                 int id,
                 int *vehicleCount)
{
    this->VehicleQuantity = VehicleQuantity;
    this->vehiclesInBridge = vehiclesInBridge;
    this->consoleAvailable = consoleAvailable;
    this->direction = direction;
    this->id = id;
    this->vehicleCount = vehicleCount;
}

void Vehicle::run()
{
    getIn();

    vehiclesInBridge->acquire();
    (*vehicleCount)++;
    crossBridge();
    vehiclesInBridge->release();

    getOut();
}

void Vehicle::getIn()
{
    consoleAvailable->acquire();
    std::cout << id << " entra   " << (direction==1 ? "<---" : "--->") << "\n";
    consoleAvailable->release();
}


void Vehicle::crossBridge()
{
    consoleAvailable->acquire();
    std::cout << id << " cruza   " << (direction==1 ? "<---" : "--->")  << "\t En el puente: " << *vehicleCount << "\n";
    consoleAvailable->release();
}

void Vehicle::getOut()
{
    consoleAvailable->acquire();
    std::cout << id << " sale   " << (direction==1 ? "<---" : "--->") << "\n";
    consoleAvailable->release();
}
