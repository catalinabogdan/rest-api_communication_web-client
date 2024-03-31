# rest-api_communication_web-client

I have implemented the client program in C, which allows interaction with a REST API, makes HTTP requests, and processes their responses. A socket for communication is opened, and in an infinite loop, user keyboard inputs are analyzed.

The "register" command allows registering a new client on the server. The user needs to input a username and a password, which will be transmitted to the server via a POST request, including a JSON containing the credentials. If there is no user already registered with the same name, a success message is displayed; otherwise, registration cannot be completed, and an error message is displayed.

The "login" command is similar to the "register" command, but it requires the user to be already registered, the password to be correct, and, like in the case of registration, the credentials cannot contain spaces (verified with the "has_no_space" function included in helpers.c) and cannot be left incomplete. This case will result in a connection error. If login is successful, the length of the corresponding cookie is saved, initialized to 0.

"Enter_library" generates a GET request, requesting permission to enter the user's library, sending the cookie received at login to the server. This cannot be done without the user being logged in. The request returns a unique token, whose length we update and save, to be useful in subsequent checks. Along with entering the library, the user is allowed to view all the books they own or a particular one, add and delete a book.

The "get_books" command generates a GET request to the server, which returns an array of JSON objects containing the id and title of the books. They are displayed using functions from the parson library. This is not possible without access to the library.

"Get_book", similar to get_books, requests information about a particular book, by id. If the entered id contains anything other than numbers or if that book does not exist in the library, an error message is displayed. Otherwise, all information about the book is printed, also using the parsing of the JSON, which has the value of the string with all the particularities returned by the server. The command is not possible without access to the library.

"Add_book" performs a POST request, in which the entered data about the new book is transmitted. The validity of this data is very important. If no information is provided or if the number of pages is in an invalid format (contains anything other than numbers, is a positive number - verification is done using the "has_no_chars" function included in helpers.c), the user is given 2 chances to enter the data correctly, and if the input is still wrong, an error message is displayed; otherwise, the book is successfully added.

"Delete_book" performs a DELETE request, implemented in requests.c, which requests the deletion of a book by its id. Similar to get_book, the validity of the entered id is checked, and it is necessary for the user to have access to the library.

"Logout" disconnects a user, resets the lengths and values of the token and cookie, so that operations such as add, delete, or get books cannot be performed because the user is no longer logged in.

"Exit" is the command that interrupts the infinite loop and closes the communication socket with the server.
