import rclpy
from rclpy.node import Node
from std_msgs.msg import Int32


class CounterSubscriber(Node):
    def __init__(self):
        super().__init__('counter_subscriber')

        self.subscription = self.create_subscription(
            Int32,
            'counter',
            self.receive_counter,
            10
        )

    def receive_counter(self, message):
        self.get_logger().info(
            f'Received: {message.data}'
        )


def main(args=None):
    rclpy.init(args=args)

    node = CounterSubscriber()

    rclpy.spin(node)

    node.destroy_node()
    rclpy.shutdown()


if __name__ == '__main__':
    main()