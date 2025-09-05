#include <iostream>
#include <vector>
#include <string>

using namespace std;

int grid[22][10];


int main()
{
    for (int i = 0; i < 22; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            grid[i][j] = 0;
        }
    }
    for (int i = 0; i < 22; i++)
    {
        for (int j = 0; j < 10; j++)
        {
            cout << grid[i][j] << " ";
        }
    }
    return 0;
}
