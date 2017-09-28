#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cfloat>
#include <cstring>
#include <algorithm>
#include <ctime>

using namespace std;

// define the constance
const int INF = 0x3f3f3f3f;
const int MAXN = 1010;
const int MAXC = 1000;
const int MAXL = 50;

const float eliteratio = 0.01; // the ratio of elites
const int SA = 1000; // the number of status
const int randrate = 0; // randomized rate = (1-randrate)
const float variationrate = 0.05; // rate of variation

int stot, ctot;
int warehousenum;
float RATE;
long long t;

// define the structure of cities and states
struct S{
    char name[MAXL]; // state's name
    int citylist[MAXN]; // list of city STATUS this state
    int ttot;
}state[55];// subscripts are state number

struct C{
    char name[MAXL]; // city's name
    int statenumber; // the number of state which the city belong to
    int time[MAXN]; // times STATUS minute which is less than 24 hours
    int to[MAXN];
    int tot;
}city[MAXN]; // subscripts are city number

struct STATUS{
    int n;
    int times[MAXN];
    bool warehouse[MAXN];
    int covered;
    int nocovered;
}status[SA + 10];// elements of each status

bool ifsamearray(char * a, char * b)
{
    int lena = strlen(a);
    int lenb = strlen(b);
    if(lena != lenb) return false;
    for(int i = 0; i < lena; i ++)
        if(a[i] != b[i]) return false;
    return true;
}

int statename2num(char * name)
{
    for(int i = 0; i < 53; i ++)
        if(name[0] == state[i].name[0] && name[1] == state[i].name[1])
            return i;
    return -1;
}

bool ifmainland(int citynum)
{
    char name[2];
    name[0] = state[city[citynum].statenumber].name[0];
    name[1] = state[city[citynum].statenumber].name[1];
    if(name[0] == 'H' && name[1] == 'I') return false;
    if(name[0] == 'P' && name[1] == 'R') return false;
    if(name[0] == 'A' && name[1] == 'K') return false;
    return true;
}

int cityname2num(char * name)
{
    int len = strlen(name);
    for(int i = 0; i < MAXC; i ++)
    {
        if(len != strlen(city[i].name)) continue;
        bool ifmatched = true;
        for(int j = 0; j < len; j ++)
            if(name[j] != state[i].name[j]) ifmatched = false;
        if(ifmatched) return i;
    }
    return -1;
}

int time2minute(char * time)
{
    int len = strlen(time), i;
    int hour[2] = {0,0}, minute[2]= {0,0};
    for(i = 1; time[i] != ':'; i ++)
    {
        hour[i - 1] = time[i] - 48;
    }
    i ++;
    for(int j = 0; time[i] != ':'; i ++, j ++)
        minute[j] = time[i] - 48;
    return hour[0]*600 + hour[1]*60 + minute[0]*10 + minute[1];
}

void preprocess()
{
    FILE * timedata = fopen("US.csv", "r"); // open the data file
    memset(city, 0, sizeof(city));
    char tempchar, temp[MAXL]; // temporary input character
    int i = 0, j = 0, k = 1; // temporary loop variables
    while(fscanf(timedata, "%c", &tempchar) != EOF)
    {
    back:
        if(tempchar == '\n') continue;
        if(tempchar == ','){
            if(j == 0 && i != 0){
                char tempsname[2], tempcname[MAXL];
                for(int l = 1; l < k - 3; l ++) // copy the city name
                    tempcname[l - 1] = temp[l];
                for(int l = k - 2, i1 = 0; i1 < 2; l ++, i1 ++) // copy the state name
                    tempsname[i1] = temp[l];

                for(int l = 0; l < k - 4 ; l ++)
                    city[ctot].name[l] = tempcname[l];
                int tempsnum = statename2num(tempsname);
                if(tempsnum == -1){
                    for(int l = 0; l < 2; l ++)
                        state[stot].name[l] = tempsname[l];
                    state[stot].citylist[state[stot].ttot] = i - 1; // create a new state number and add this city to its state
                    city[ctot].statenumber = stot; // write the state number to city structure
                    state[stot].ttot ++;
                    stot ++;
                }
                else
                {
                    city[ctot].statenumber = tempsnum; // write the state number to city structure
                    state[tempsnum].citylist[state[tempsnum].ttot ++] = i - 1; // add the city number to its state
                }
                ctot ++;
            }
            if(i != j && i > 0 && j > 0)
            {
                char temptime[MAXL];
                for(int l = 1; l < k; l ++)
                    temptime[l] = temp[l];
                int minute = time2minute(temptime);
                if(minute <= (1440 / RATE)){
                    city[i - 1].time[j - 1] = minute;
                    city[i - 1].to[city[i - 1].tot] = j - 1;
                    city[i - 1].tot ++;
                }
            }
            if(i == j && i > 0 && j > 0)
            {
                city[i - 1].time[j - 1] = 0;
                city[j - 1].time[i - 1] = 0;
            }
            i ++;
            k = 1;
            memset(temp, 0, sizeof(temp));
            continue;
        }
        if(tempchar == ';'){
            i = 0;
            j ++;
            continue;
        }
        temp[k] = tempchar; // write the temporary character to array
        k ++;
    }
}

// Main Algorithm Begins
int cmp(const void * a , const void * b )
{
    struct STATUS *c = (STATUS *)a;
    struct STATUS *d = (STATUS *)b;
    if(c -> nocovered != d -> nocovered) return c -> nocovered - d -> nocovered;
    else return c -> covered - d -> covered;
}

void emptytimes()
{
    for(int i = 0; i < SA; i ++)
    {
        memset(status[i].times, 0, sizeof(status[i].times));
        status[i].covered = INF;
        status[i].nocovered = INF;
    }
}
void calculatevalue()
{
    for(int i = 0; i < SA; i ++)
    {
        status[i].nocovered = 0;
        status[i].covered = 0;
        for(int j = 0; j < MAXC; j ++)
            if(status[i].times[j] > 1)
                status[i].covered += (status[i].times[j] - 1);

        for(int j = 0; j < MAXC; j ++)
            if(status[i].times[j] == 0)
                status[i].nocovered ++;
    }
}

void calculatetimes()
{
    for(int i = 0; i < SA; i ++)
    {
        for(int j = 0; j < MAXC; j ++)
            if(status[i].warehouse[j])
                for(int k = 0; k < city[j].tot; k ++)
                    status[i].times[city[j].to[k]] ++;
    }
}

void initialization()
{
    srand((unsigned)time(NULL));
    memset(status, 0, sizeof(status));
    for(int i = 0; i < SA; i ++)
    {
        int tempnum = 1; // temporary the number of warehouses
        while(tempnum <= warehousenum)
        {
            int citynum = rand() % 1000;// ramdomly initialize warehouse location
            if(!status[i].warehouse[citynum] && ifmainland(citynum)){
                tempnum ++;
                status[i].warehouse[citynum] = true;
            }
        }
    }
    // debug
    //    for(int i = 0; i < 1000; i ++)
    //    	cout << status[4324].warehouse[i];
    //   	cout << endl;
    //    for(int i = 0; i < 1000; i ++)
    //   	cout << status[345].warehouse[i];
    calculatetimes();
    calculatevalue();
}

void mate()
{
    srand((unsigned)time(NULL));
    int temptimes = 0, num1 = 0, num2 = 0;
    int maxtimes = rand() % SA * (1 - 2*randrate) + SA * randrate;
    while(temptimes <= maxtimes)
    {
        num1 = num2;
        while(num1 == num2 || num1 < SA * eliteratio || num2 < SA * eliteratio)
        {
            num1 = rand() % SA;
            num2 = rand() % SA;
        }
        int tempnum = rand() % SA;
        while(!ifmainland(tempnum)) tempnum = rand() % SA;
        if(status[num2].warehouse[tempnum] != status[num1].warehouse[tempnum]){
            swap(status[num1].warehouse[tempnum], status[num2].warehouse[tempnum]);
            temptimes ++;
        }
    }
    calculatetimes();
    calculatevalue();
}

void variation()
{
    int temptimes = 0;
    int maxtimes = SA * variationrate;
    for(int i = 0; i < SA * variationrate; i ++)
    {
        int tempnum = rand() % SA;
        bool better = false;
        while(!better)
        {
            // Debug
        }
    }
}

int numwh(int num) // return the number of warehouses in this status
{
    int ans = 0;
    for(int i = 0; i < MAXC; i ++)
        if(status[num].warehouse[i]) ans ++;
    return ans;
}

void GA()
{
    FILE * data = fopen("citydata.csv", "w");
    FILE * output = fopen("citylist.txt", "w");
    memset(status, 0, sizeof(status));
    initialization();
    int last = INF, last2 = INF, number = 1;
    while(1)
    {
        t ++;
        for(int i = 0; i < SA; i ++)
            status[i].n = numwh(i);
        qsort(status, SA, sizeof(STATUS), cmp);
        mate();
        int tempnum = numwh(0);
        cout << status[0].nocovered << " " << tempnum << " " << t << " " << RATE <<endl;
        if(last > tempnum || last2 > status[0].nocovered)
        {
            fprintf(output, "test number = %d\n", number);
            for(int i = 0; i < 1000; i ++)
                if(status[0].warehouse[i]){
                    for(int j = 0; j < strlen(city[i].name); j ++)
                        fprintf(output, "%c", city[i].name[j]);
                    fprintf(output, "\n");
                }
            fprintf(output, "\n");

            fprintf(data, "%d,%d,%d,%d\n", number, t, tempnum, status[0].nocovered);

            last2 = status[0].nocovered;
            last = tempnum;
            number ++;
        }
    }
}

int main()
{
    cin >> RATE;
    srand((unsigned)time(NULL));
    warehousenum = rand()%10 + 30;
    preprocess();
    /*   debug
     int a = 0;
     for(int i = 0; i < 1000; i ++){
    	if(!ifmainland(i)) continue;
    	cout << a << " "<<city[i].tot << " ";
    	for(int j = 0; j < strlen(city[i].name); j ++)
	  		cout << city[i].name[j];
	  		cout << endl;
	  		a ++;
     }
     */
    GA();
    // debug
    //	for(int i = 0; i < SA; i ++)
    //		cout << status[i].nocovered << " " << numwh(i) << endl;
    //	for(int i = 0; i < 1000; i ++)
    //	{
    //		cout << status[0].times[i] << endl;
    //	}
    return 0;
}
