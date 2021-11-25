//
// void is_palindrome(int arr[], int n)
// {
//     // Initialise flag to zero.
//     int flag = 0;
//
//     // Loop till array size n/2.
//     for (int i = 0; i <= n / 2 && n != 0; i++) {
//
//         // Check if first and last element are different
//         // Then set flag to 1.
//         if (arr[i] != arr[n - i - 1]) {
//             flag = 1;
//             break;
//         }
//     }
//
//     // If flag is set then print Not Palindrome
//     // else print Palindrome.
//     if (flag == 1)
//         cout << "Not Palindrome";
//     else
//         cout << "Palindrome";
// }
