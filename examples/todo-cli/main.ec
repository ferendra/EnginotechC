// Todo CLI Example - EnginotechC++
// Demonstrates basic CLI operations

struct Todo {
    id: int,
    title: string,
    done: bool,
}

fn create_todo(id: int, title: string) -> Todo {
    return Todo{id: id, title: title, done: false};
}

fn list_todos() -> int {
    print("--- Todo List ---");
    print("No todos yet");
    print("---");
    return 0;
}

fn main() -> int {
    let todo = create_todo(1, "Learn EnginotechC++");
    print("Created todo: " + todo.title);
    
    list_todos();
    
    print("\nTodo CLI demo complete!");
    return 0;
}
