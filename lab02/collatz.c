int collatz_conjecture(int input)
{
    if (input % 2 == 0)
    {
        return input / 2;
    }
    else
    {
        return 3 * input + 1;
    }
}

int test_collatz_convergence(int input, int max_iter, int *steps)
{
    int i = 0;
    while (i < max_iter)
    {
        input = collatz_conjecture(input);
        steps[i] = input;
        if (input == 1)
        {
            return i;
        }
        i++;
    }
    return 0;
}
