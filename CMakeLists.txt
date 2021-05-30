cmake_minimum_required(VERSION 3.17)
project(OSHW2 C)

set(CMAKE_C_STANDARD 99)

add_executable(OSHW2 concurrent_list.c )

#int main(){
#
#list* l = create_list();
#insert_value(l, 3);
#insert_value(l, 1);
#insert_value(l, 5);
#insert_value(l, 2);
#insert_value(l, 7);
#insert_value(l, 4);
#insert_value(l, 12);
#remove_value(l,1);
#
#
#remove_value(l,12);
#remove_value(l,4);
#//    print_list(l);
#
#remove_value(l,2);
#remove_value(l,5);
#
#print_list(l);
#
#return 0;
#}