#include "MainFlow.h"
#include <iostream>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include "Client.h"
#include "Driver.h"
class ThreadClient{
public:
    Tcp* sock;
    MainFlow* flow;
    int clientDescriptor;
    ThreadClient(Tcp* s, MainFlow* f){
        this->sock = s;
        this->flow = f;
        this->clientDescriptor;
    }
    ~ThreadClient(){}
};

//
// This will handle connection for each client
//
void *connectionHandler(void *socket_desc) {
    ThreadClient* handler = (ThreadClient*)socket_desc;
    char buffer[4096];
    memset(buffer,0,sizeof(buffer));
    int dataSize = handler->sock->reciveData(buffer, 4096,handler->clientDescriptor);
    BOOST_LOG_TRIVIAL(debug)<<"**thread - received driver data from client!**"<<endl;
    Driver *driver;
    boost::iostreams::basic_array_source<char> device(buffer, dataSize);
    boost::iostreams::stream<boost::iostreams::basic_array_source<char> > s2(device);
    boost::archive::binary_iarchive ia(s2);
    ia >> driver;
    cout<<driver->getId()<<endl;
    cout<<driver->getAge()<<endl;
    cout <<"zise before set"<< handler->flow->boolVector.size()<<endl;
    handler->flow->setBoolVectorAt(driver->getId(),true);
    driver->setCurrentPoint(handler->flow->getGrid()->getNode(Point(0,0)));
    Node* currentPoint=driver->getCurrentPoint();
    //find the taxi

    BOOST_LOG_TRIVIAL(debug)<<"thread - find the right taxi!"<<endl;
    Cab* taxi = handler->flow->getCab(driver->getTaxiId());

    //send the taxi case number

    BOOST_LOG_TRIVIAL(debug)<<"**thread - send 2!**"<<endl;
    handler->sock->sendData("2",handler->clientDescriptor);
    //send the taxi

    std::string serial_str;
    boost::iostreams::back_insert_device<std::string> inserter(serial_str);
    boost::iostreams::stream<boost::iostreams::back_insert_device<std::string> > s(inserter);
    boost::archive::binary_oarchive oa(s);
    oa << taxi;
    s.flush();
    cout<<serial_str<<endl;
    handler->sock->sendData(serial_str,handler->clientDescriptor);
    BOOST_LOG_TRIVIAL(debug)<<"**thread - send taxi!**"<<endl;
    //serialize and send current point
    std::string serial_str3;
    boost::iostreams::back_insert_device<std::string> inserter3(serial_str3);
    boost::iostreams::stream<boost::iostreams::back_insert_device<std::string> > s4(inserter3);
    boost::archive::binary_oarchive oc(s4);
    oc << currentPoint;
    s4.flush();
    handler->sock->sendData(serial_str3,handler->clientDescriptor);
    BOOST_LOG_TRIVIAL(debug)<<"**thread - send current point!**"<<endl;
    //receive driver *with* his cab object
    memset(buffer,0,sizeof(buffer));
    dataSize = handler->sock->reciveData(buffer,4096,handler->clientDescriptor);
    boost::iostreams::basic_array_source<char> device6(buffer,dataSize);
    boost::iostreams::stream<boost::iostreams::basic_array_source<char> > s7(device6);
    boost::archive::binary_iarchive ih(s7);
    delete driver;
    ih >> driver;
    BOOST_LOG_TRIVIAL(debug)<<"**thread - received driver with taxi from client!**"<<endl;
    driver->setClientDescriptor(handler->clientDescriptor);
    handler->flow->getTaxiCenter()->addDriver(driver);
    //driver->setMyTripInfo(NULL);
    //handler->flow->setBoolVectorAt(driver->getId(),true);

    while (handler->flow->getMission()!=7) {
        //handler->flow->setBoolVectorAt(driver->getId(),true);

        //BOOST_LOG_TRIVIAL(debug)<<"**thread - changed to true!**"<<endl;
        switch (handler->flow->getMission()) {
            case (10): {
                BOOST_LOG_TRIVIAL(debug)<<"**thread - hi!!**"<<endl;
                //if there is a trip

                //BOOST_LOG_TRIVIAL(debug)<<"**thread - changed to false!**"<<endl;
                //handler->flow->setBoolVectorAt(driver->getId(),false);
                    //initialize the trip's information
                    int thereIsTrip = handler->flow->checkIfTimeToTrip(handler->flow->getTime(),driver->getId());
                    BOOST_LOG_TRIVIAL(debug)<<"therIsTrip "<<thereIsTrip<<endl;
                    BOOST_LOG_TRIVIAL(debug)<<"time "<<handler->flow->getTime()<<endl;
                //driver->getMyTripInfo()->getStartingP()->getPoint().printPoint();
                    handler->flow->getTaxiCenter()->startDriving(driver->getId());
                    //a trip needs to start
                    if (thereIsTrip!=-1){
                        BOOST_LOG_TRIVIAL(debug) << "thread - we got a trip!" << endl;
                        //find destPoint
                        Node *destPoint = handler->flow->getTaxiCenter()->getTripsVector().at(thereIsTrip)
                                ->getEndingP();
                        //sent the destination point case number
                        handler->sock->sendData("4", driver->getClientDescriptor());
                        //send the destination point

                        std::string serial_str3;
                        boost::iostreams::back_insert_device<std::string> inserter3(serial_str3);
                        boost::iostreams::stream<boost::iostreams::back_insert_device
                                <std::string> > s4(inserter3);
                        boost::archive::binary_oarchive oc(s4);
                        oc << destPoint;
                        BOOST_LOG_TRIVIAL(debug)<<"thread - sent dest point!"<<endl;
                        s4.flush();
                        handler->sock->sendData(serial_str3, driver->getClientDescriptor());
                        thereIsTrip=true;
                        //sent the tripInfo case number
                        handler->sock->sendData("3",driver->getClientDescriptor());
                        //find the trip info
                        TripInfo* tripInfo = handler->flow->getTaxiCenter()->getTripsVector().at(thereIsTrip);

                        //send the tripInfo
                        std::string serial_str2;
                        boost::iostreams::back_insert_device<std::string> inserter2(serial_str2);
                        boost::iostreams::stream<boost::iostreams::back_insert_device
                                <std::string> > s3(inserter2);
                        boost::archive::binary_oarchive ob(s3);
                        ob << tripInfo;
                        BOOST_LOG_TRIVIAL(debug)<<"thread - send trip info!"<<endl;
                        s3.flush();
                        handler->sock->sendData(serial_str2,driver->getClientDescriptor());
                    }

                    //send new location (Node*)
                    Node *newLocation = driver->getCurrentPoint();
                    //in case of a change in the location
                    if (currentPoint != newLocation) {
                        //sent the new location case number
                        BOOST_LOG_TRIVIAL(debug) << "thread - send new location case number" << endl;
                        handler->sock->sendData("5", driver->getClientDescriptor());
                        std::string serial_str4;
                        boost::iostreams::back_insert_device<std::string> inserter4(serial_str4);
                        boost::iostreams::stream<boost::iostreams::back_insert_device
                                <std::string> > s5(inserter4);
                        boost::archive::binary_oarchive od(s5);
                        od << newLocation;
                        s5.flush();

                        BOOST_LOG_TRIVIAL(debug) << "thread - send the new location!" << endl;
                        handler->sock->sendData(serial_str4, driver->getClientDescriptor());
                        //update (local) current point
                        currentPoint = newLocation;
                    }
                }
            }

    }
    BOOST_LOG_TRIVIAL(debug)<<"thread - print taxi id!"<<endl;
    BOOST_LOG_TRIVIAL(debug)<<driver->getTaxiId()<<endl;
    BOOST_LOG_TRIVIAL(debug)<<"thread - bye bye thread"<<endl;
    pthread_exit(socket_desc);
}

MainFlow::MainFlow(){
    this->myTaxiCenter = new TaxiCenter();
    this->grid;
    this->cabsVector;
    this->threads;
    this->boolVector;
    this->mission;
    this->time=0;

}

MainFlow::~MainFlow() {
 delete (myTaxiCenter);
    delete (grid);
for(int i=0;i<cabsVector.size();++i){
	delete cabsVector[i];
}
}

TaxiCenter* MainFlow::getTaxiCenter(){
    return this->myTaxiCenter;
}

vector<Cab*> MainFlow::getCabsVector(){
    return this->cabsVector;
}

void MainFlow::createCab(int idTaxi,int typeOfTaxi,char manufacturer,char color){
    Manufacturer taxiManufacturer= Manufacturer (manufacturer);
    Color taxiColor = Color (color);
    if (typeOfTaxi==1){
        StandardCab* newStandardCab = new StandardCab(idTaxi,typeOfTaxi,taxiManufacturer,taxiColor);
        cabsVector.push_back(newStandardCab);
    }
    else{
        LuxuryCab* newLuxuryCab = new LuxuryCab(idTaxi,typeOfTaxi,taxiManufacturer,taxiColor);
        cabsVector.push_back(newLuxuryCab);
    }
}

void MainFlow::createTripInfo(int tripId, int xStartPoint,int yStartPoint, int xEndPoint, int yEndPoint,
                                   int numOfpassn, double tariff, int timeOfStart){
    TripInfo* newTrip = new TripInfo(tripId,grid->getNode(Point(xStartPoint,yStartPoint)),
                                     grid->getNode(Point(xEndPoint,yEndPoint)),numOfpassn,tariff, timeOfStart);
    this->myTaxiCenter->addTrip(newTrip);
}

void MainFlow::createGrid(int gridX,int gridY){
    this->grid = new Grid(gridX,gridY);
    this->myTaxiCenter->setMyGrid(this->grid);
    }
Grid* MainFlow::getGrid(){
    return this->grid;
}

Cab* MainFlow::getCab(int texiId) {
    for(int i=0;i<this->cabsVector.size();i++){
        if(this->cabsVector[i]->getId()==texiId){
            return this->cabsVector[i];
        }
    }
}

int MainFlow::checkIfTimeToTrip(int time,int driverId){
    for(int i=0; i<this->myTaxiCenter->getTripsVector().size();i++) {
        if(this->myTaxiCenter->getTripsVector().at(i)!=NULL&&
                this->myTaxiCenter->getTripsVector().at(i)->getTimeOfStart()==time&&
                this->myTaxiCenter->getTripsVector().at(i)->getHaveDriver()== false){
            this->myTaxiCenter->connectDriversToTrips(i,driverId);
            return i;
        }
    }
    return -1;
}

void MainFlow::mainFlow(int portNum){
    int gridXAxe,gridYAxe,numOfObstacles;
    int xPoint,yPoint;
    char dummy;
    bool mainBool=false;
    char buffer[4096];
    Tcp* socket= new Tcp(true,portNum);
    socket->initialize();

    cout<<"size in initialize"<<boolVector.size()<<endl;
    //entered the size of the grid (map)
    cin >> gridXAxe >> gridYAxe;
    this->createGrid(gridXAxe,gridYAxe);
    //entered the numbers of the obstacles that will be at the map
    cin >> numOfObstacles;
    //add the obstacles to the map
    while(numOfObstacles>0){
        cin >> xPoint >> dummy >> yPoint;
        this->getGrid()->getNode(Point(xPoint,yPoint))->setIsObstacle();
        numOfObstacles--;
    }
    int numOfDrivers=0;
    do{
        cin>>this->mission;
        switch (this->mission) {
            //this mission is for creating and adding a new driver to the game
            case 1: {
                cin >> numOfDrivers;
                for(int i =0  ; i < numOfDrivers; i++){
                    //receive the driver
                    ThreadClient* threadHandler = new ThreadClient(socket, this);
                    boolVector.push_back(true);
                    cout<<"size after push1 "<<boolVector.size()<<endl;
                    threadHandler->clientDescriptor = threadHandler->sock->acceptOneClient();
                    cout<<"size after push 2"<<boolVector.size()<<endl;
                    BOOST_LOG_TRIVIAL(debug)<<"mainflow - server accepted client!"<<endl;
                    cout<<"size after push 3"<<boolVector.size()<<endl;
                    pthread_create(&threads[i], NULL, connectionHandler, threadHandler);
                    cout<<"size after push 4"<<boolVector.size()<<endl;
                }
                cout<<"size after push after loop "<<boolVector.size()<<endl;
                break;
            }
                //this mission is for creating and adding a new trip to the game
            case 2: {
                int tripId, xStartPoint, yStartPoint, xEndPoint, yEndPoint, numOfpassn,timeOfStart;
                double tariff;
                //initialize the nodes' 'visited' values
                this->getGrid()->initializeBeforeTrip();
                cin >> tripId >> dummy >> xStartPoint >> dummy >> yStartPoint >> dummy >>
                    xEndPoint >> dummy >> yEndPoint >>
                    dummy >> numOfpassn >> dummy >> tariff >> dummy >> timeOfStart;
                this->createTripInfo(tripId, xStartPoint, yStartPoint, xEndPoint, yEndPoint,
                                     numOfpassn, tariff, timeOfStart);
                break;
            }
            //this mission is for creating and adding a new vehicle to the game
            case 3: {
                int idTaxi;
                int typeOfTaxi;
                char manufacturer, color;
                cin >> idTaxi >> dummy >> typeOfTaxi >> dummy >> manufacturer >> dummy >> color;
                this->createCab(idTaxi, typeOfTaxi, manufacturer, color);
                break;
            }
                // this mission is for printing the current point of specific driver
            case 4: {
                int idToGet;
                cin >> idToGet;
                Driver *driverCurrentPoint = this->getTaxiCenter()->getDriverById(idToGet);
                Node *currentPoint = driverCurrentPoint->getCurrentPoint();
                currentPoint->getPoint().printPoint();
                break;
            }
            //this mission increase the time by one
            case 9: {
                int i;
                while (!this->finish()) { /*cout<< "ifat is right"<<endl; */ }
                //waiting for all the threads
                //this->mission = 10;
                ///mainBool=false;
                /*while(mainBool==false){
                    mainBool=setMainBool();
                }*/
                BOOST_LOG_TRIVIAL(debug)<<"mainflow - case9!" << endl;
                time++;
                for(int i = 0;i<boolVector.size();i++){
                    boolVector[i] = false;
                }
                this->mission = 10;
                break;
            }
        }
        //if the user press 7 the game will end
    }while(this->mission!=7);
    Driver* driver = this->myTaxiCenter->getDriverById(0);
    //sent the delete case number
    socket->sendData("7",driver->getClientDescriptor());
}

void MainFlow::setBoolVectorAt(int i,bool state){
    this->boolVector[i]=state;
}
int MainFlow::getMission(){
    return this->mission;
}
int MainFlow::getTime(){
    return this->time;
}
bool MainFlow::setMainBool(){
    for(int i=0;i<boolVector.size();i++) {
        if (boolVector[i] == false)
            return false;
    }
    return true;
}

bool MainFlow::finish(){
    cout<<"size "<<boolVector.size()<<endl;
    for(int i=0; i<boolVector.size(); i++){
        if(boolVector[i] == false)
            return false;
    }
    return true;
}