#pragma once
#include <iostream>

template<typename C>
void printContainer(const C& container, const std::string& containerName,
                    int maxItemNumber)
{
    using std::cout;

    cout << containerName << "\n";
    int itemsToPrint = std::min(maxItemNumber, (int)container.size() );
    for (int i=0; i<itemsToPrint; i++){
        cout << container[i] << " ";
    }
    cout << "\n";
}

template<typename C>
void printContainer(const C& container,
                    const std::string& containerName)
{
    printContainer(container, containerName, container.size() );
}
