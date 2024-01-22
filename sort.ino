
//сортировка для сканирования
void sort_2(int mas[n], String mas2[n], int n)
{
  for (int startIndex = 0; startIndex < n - 1; startIndex+=1)
  {
    int smallestIndex = startIndex;
    for (int currentIndex = startIndex + 1; currentIndex < n; ++currentIndex)
    {
      if ((mas[currentIndex]) < (mas[smallestIndex]))
      smallestIndex = currentIndex;
    }
    int temp=mas[startIndex];
    mas[startIndex]=mas[smallestIndex];
    mas[smallestIndex]=temp;
    String temp2=mas2[startIndex];
    mas2[startIndex]=mas2[smallestIndex];
    mas2[smallestIndex]=temp2;
  }
  for (int i = 0; i < n / 2; i++) 
  {
    int tmp = mas[i];
    mas[i] = mas[n - i - 1];
    mas[n - i - 1] = tmp;
    String tmp2 = mas2[i];
    mas2[i] = mas2[n - i - 1];
    mas2[n - i - 1] = tmp2;
  }
}
