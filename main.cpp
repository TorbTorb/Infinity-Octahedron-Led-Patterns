#include <Arduino.h>
#include <FastLED.h>
#include <vector>
#include <algorithm>
#include "math.h"

#define NUM_LEDS 25*12      //12 edges a 25 leds

#define DATA_PIN 21

//the array of leds
CRGB leds[NUM_LEDS];
class Vertex;
class Edge;
Edge* edgeList[12];
Vertex* vertexList[6];


class Edge {
    private:
        int position;
        Vertex* fromVertex = nullptr;
        Vertex* toVertex = nullptr;
    public:
        Edge(int pos); //, Vertex* fromVertex=nullptr, Vertex* toVertex=nullptr);
        ~Edge() {};
        void setVertices(Vertex* from, Vertex* to);
        int getStartIndex();
        int getEndIndex();
        int getPos();
        Vertex* getStartVertex();
        Vertex* getEndVertex();
};
Edge::Edge(int pos): position(pos) {}

void Edge::setVertices(Vertex* from, Vertex* to) {
    fromVertex = from;
    toVertex = to;
}

int Edge::getPos() {
    return position;
}
int Edge::getStartIndex() {
    return position*25;
}

int Edge::getEndIndex() {
    return position*25+24;
}
Vertex* Edge::getStartVertex() {
    return fromVertex;
}
Vertex* Edge::getEndVertex() {
    return toVertex;
}


class Vertex {
    private:
        int ID;
        Edge* connected[4];
        int coordinates[3];
    public:
        Vertex(int ID, int x, int y, int z, Edge* edge1, Edge* edge2, Edge* edge3, Edge* edge4);
        ~Vertex();
        std::vector<int> getEdges(); //returns all indeces of edges, led index, and direction ( vector<int> = [<edge_ID_0>, <led_index_0>, <direction_0>, ...])
        int getID();
        int* getCoordinates();
};
Vertex::Vertex(int ID, int x, int y, int z, Edge* edge1, Edge* edge2, Edge* edge3, Edge* edge4): ID(ID) {
        coordinates[0] = x;
        coordinates[1] = y;
        coordinates[2] = z;

        connected[0] = edge1;
        connected[1] = edge2;
        connected[2] = edge3;
        connected[3] = edge4;
    }

//returns all indeces of edges, led index, and direction ( vector<int> = [<edge_ID_0>, <led_index_0>, <direction_0>, ...])
std::vector<int> Vertex::getEdges() {
    std::vector<int> out;
    int index, ledIndex, direction;
    for (auto &&edge : connected) {
        index = edge->getPos();
        if(ID == edge->getStartVertex()->getID()) {     //self is start Vertex of edge
            ledIndex = edge->getStartIndex();
            direction = 1;
        } else {
            ledIndex = edge->getEndIndex();
            direction = -1;
        }
    out.push_back(index);
    out.push_back(ledIndex);
    out.push_back(direction);
    }
    return out;
}

int Vertex::getID() {
    return ID;
}

int* Vertex::getCoordinates() {
    return coordinates;
}


//check if an edge with given direction is in edge list of the format given by Vector::getEdges 
bool inEdgeList(std::vector<int> edgeListe, int edgeID, int direction) {
    for (int i = 0; i < edgeListe.size(); i+=3) //check every third element
    {
        if (edgeListe[i] == edgeID && edgeListe[i+2] == direction) {
            return true;
        }
    }
    return false;
}

//test whether a vector contains a specific element
template <typename T>
const bool contains(std::vector<T>& Vec, const T& Element ) 
{
    if (std::find(Vec.begin(), Vec.end(), Element) != Vec.end())
        return true;

    return false;
}

//modifiers coords to container the x y z values of the led
void get3dPosition(int ledIndex, float coords[3]) {
    int edgeIndex = ledIndex / 25;

    int* vertexStart = edgeList[edgeIndex]->getStartVertex()->getCoordinates();
    int* vertexEnd = edgeList[edgeIndex]->getEndVertex()->getCoordinates();

    float interpolationIndex = (ledIndex%25) / 25.f;
    for (int i = 0; i < 3; i++)
    {
        coords[i] = vertexStart[i] * (1.f-interpolationIndex)  +  vertexEnd[i] * interpolationIndex;
    }
}


//start at node 1 and go outwards
void pattern1() {   
    FastLED.clear();
    std::vector<int> currentEdges;
    std::vector<Vertex*> currentVertices = {vertexList[1]};
    std::vector<int> edgeRes;
    for (int u = 0; u < 2; u++) {       //add all edges to list 
        //currentEdges.clear();
        for (auto &&vertex : currentVertices) {
            edgeRes = vertex->getEdges();
            for (int i = 0; i < 12; i+=3) {
                if (!inEdgeList(currentEdges, edgeRes[i], edgeRes[i+2])) {       // not there yet
                    currentEdges.push_back(edgeRes[i]);       //edge index
                    currentEdges.push_back(edgeRes[i + 1]);   //led index
                    currentEdges.push_back(edgeRes[i + 2]);   //direction
                }
            }
        }

        
        for (int i = 0; i < currentEdges.size(); i+=3) {
            if (!contains(currentVertices, edgeList[currentEdges[i]]->getStartVertex())) {
                currentVertices.push_back(edgeList[currentEdges[i]]->getStartVertex());
            }
            if (!contains(currentVertices, edgeList[currentEdges[i]]->getEndVertex())) {
                currentVertices.push_back(edgeList[currentEdges[i]]->getEndVertex());
            }
        }

        for (int led_i = 0; led_i < 25; led_i++) {
            for (int edge_i = 0; edge_i < currentEdges.size(); edge_i+=3) {
                leds[currentEdges[edge_i+1] + currentEdges[edge_i+2]*led_i] = CRGB(255,255,255);
            }
            fadeToBlackBy(leds, NUM_LEDS, 50);
            FastLED.show();
            delay(25);
        }
        
    }
}

//start at node 1 and 3 and go outwards with different colors
void pattern2() {   
    CHSV color = CHSV(0, 255, 255);
    while (1) {
        color.hue = (color.hue + 40)%256;
        std::vector<int> currentEdges;
        std::vector<Vertex*> currentVertices = {vertexList[1], vertexList[3]};
        std::vector<int> edgeRes;
        for (int u = 0; u < 2; u++) {       //add all edges to list 
            //currentEdges.clear();
            for (auto &&vertex : currentVertices) {
                edgeRes = vertex->getEdges();
                for (int i = 0; i < 12; i+=3) {
                    if (!inEdgeList(currentEdges, edgeRes[i], edgeRes[i+2])) {       // not there yet
                        currentEdges.push_back(edgeRes[i]);       //edge index
                        currentEdges.push_back(edgeRes[i + 1]);   //led index
                        currentEdges.push_back(edgeRes[i + 2]);   //direction
                    }
                }
            }

            
            for (int i = 0; i < currentEdges.size(); i+=3) {
                if (!contains(currentVertices, edgeList[currentEdges[i]]->getStartVertex())) {
                    currentVertices.push_back(edgeList[currentEdges[i]]->getStartVertex());
                }
                if (!contains(currentVertices, edgeList[currentEdges[i]]->getEndVertex())) {
                    currentVertices.push_back(edgeList[currentEdges[i]]->getEndVertex());
                }
            }

            for (int led_i = 0; led_i < 25; led_i++) {
                for (int edge_i = 0; edge_i < currentEdges.size(); edge_i+=3) {
                    leds[currentEdges[edge_i+1] + currentEdges[edge_i+2]*led_i] = color;
                }
                fadeToBlackBy(leds, NUM_LEDS, 50);
                FastLED.show();
                delay(25);
            }
            
        }
    }
}

//colored thing coming from each vertex
void pattern3() {   
    CHSV color = CHSV(0,255,0);
    while(1) {
        color.value = (color.value + 224) % 256;
        if (color.value == 0) {
            color.hue = (color.hue + 25) % 256;
        }

        for (int i = 0; i < sizeof(edgeList)/sizeof(Edge*); i++)
        {
            for (int pix_i = 11; pix_i >= 0; pix_i--)
            {
                leds[25*i + pix_i + 1] = leds[25*i + pix_i];
                leds[25*i + 24 - pix_i - 1] = leds[25*i + 24 - pix_i];
            }
            leds[25*i] = color;
            leds[25*i + 24] = color;
        }

        FastLED.show();
        delay(60);
    }
}

//colored dot with streak moving randomly
void pattern4() {   
    CHSV color = CHSV(0, 255,255);
    Edge* current = edgeList[0];
    Vertex* tmp;
    Edge* next;
    int direction = 1;
    int start, stop, dirnext, rand;
    while(1) {

        if(direction == 1) {
            start = current->getStartIndex();
            stop = current->getEndIndex();
            tmp = current->getEndVertex();
        } else {
            start = current->getEndIndex();
            stop = current->getStartIndex();

            tmp = current->getStartVertex();
        }
        do {
        rand = random(4);
        next = edgeList[tmp->getEdges().at(3*rand)];

        } while(next == current);
        dirnext = tmp->getEdges().at(3*rand+2);



        for (int i = start; i != stop; i += direction)
        {
            leds[i] = color;
            color.hue = (color.hue+3)%256;

            fadeToBlackBy(leds, NUM_LEDS, 25);

            FastLED.show();
            delay(30);
        }
        direction = dirnext;
        current = next;
    }
}

//rainbow the whole thing
void pattern5() {
    CHSV color = CHSV(0,255,255);
    while(1) {
        FastLED.showColor(color);
        color.hue = (color.hue+1)%256;
        delay(100);
    }
}

//moving plane
void pattern6() {
    float thresh = 0.1;
    float yPos = 0.f;
    float coords[3];
    while(1) {
        yPos = fmodf((yPos + 1.21), 2.4) - 1.2;
        for (int i = 0; i < NUM_LEDS; i++)
        {
            get3dPosition(i, coords);
            if (coords[1] < yPos + thresh && coords[1] > yPos - thresh) {
                leds[i] = CRGB::NavajoWhite;
            }
            else {
                leds[i] = CRGB::Black;
            }
        }
        FastLED.show();
        
    }
}

//growing sphere
void pattern7() {
    float size = 0.f;
    float sizeSquared;
    float coords[3];
    float distance;
    int brightness;
    while(1) {
        size = fmodf(size + 0.0003, 1.1);
        if(size < 0.62) {
            size = 0.62;
        }
        sizeSquared = size*size;
        for (int i = 0; i < NUM_LEDS; i++)
        {
            get3dPosition(i, coords);
            distance = coords[0]*coords[0] + coords[1]*coords[1] + coords[2]*coords[2];
            brightness = max(0, 255 - int(abs(distance - sizeSquared)*3000));
            leds[i] = CRGB(brightness, brightness, brightness);
        }
        FastLED.show();
        
    }
}

//rainbow color going up
void pattern8() {
    int colorOffset = 0;
    float coords[3];


    while(1) {
        colorOffset = (colorOffset + 2) % 256;
        for (int i = 0; i < NUM_LEDS; i++)
        {
            get3dPosition(i, coords);

            leds[i] = CHSV((int((coords[1]+1)*128) + colorOffset) % 256, 255, 255);
        }
        FastLED.show();
        delay(20);
        
    }
}


// This function sets up the leds and tells the controller about them
void setup() {
    Serial.flush();
    Serial.begin(9600);
    Serial.println("Start");

    FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);  // GRB ordering is typical
    FastLED.showColor(CRGB::Red);
    delay(1000);

    for (int i = 0; i < 12; i++)
    {
        edgeList[i] = new Edge(i);
    }

    //connect all the vertices and edges
    vertexList[0] =  new Vertex(0, 1, 0 ,0 , edgeList[0], edgeList[1], edgeList[3], edgeList[4]);
    vertexList[1] =  new Vertex(1, 0, -1, 0, edgeList[0], edgeList[6], edgeList[11], edgeList[5]);
    vertexList[2] =  new Vertex(2, 0, 0, 1, edgeList[1], edgeList[2], edgeList[7], edgeList[6]);
    vertexList[3] =  new Vertex(3, 0, 1, 0, edgeList[3], edgeList[9], edgeList[8], edgeList[2]);
    vertexList[4] =  new Vertex(4, 0, 0, -1, edgeList[4], edgeList[5], edgeList[10], edgeList[9]);
    vertexList[5] =  new Vertex(5, -1, 0, 0, edgeList[7], edgeList[8], edgeList[10], edgeList[11]);

    edgeList[0]->setVertices(vertexList[1], vertexList[0]);
    edgeList[1]->setVertices(vertexList[0], vertexList[2]);
    edgeList[2]->setVertices(vertexList[2], vertexList[3]);
    edgeList[3]->setVertices(vertexList[3], vertexList[0]);
    edgeList[4]->setVertices(vertexList[0], vertexList[4]);
    edgeList[5]->setVertices(vertexList[4], vertexList[1]);
    edgeList[6]->setVertices(vertexList[1], vertexList[2]);
    edgeList[7]->setVertices(vertexList[2], vertexList[5]);
    edgeList[8]->setVertices(vertexList[5], vertexList[3]);
    edgeList[9]->setVertices(vertexList[3], vertexList[4]);
    edgeList[10]->setVertices(vertexList[4], vertexList[5]);
    edgeList[11]->setVertices(vertexList[5], vertexList[1]);



    CHSV color = CHSV(0, 255, 100);
    CHSV ogColor = CHSV(color);
    while(1){
        pattern8();
    }
}



void loop() {}