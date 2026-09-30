using Godot;

// Energy Transitions
// Nat Sci Hall - OMSI
// Calico Rose
// Used to detect if someone has swiped the touchscreen.
// Reference: https://www.youtube.com/watch?v=7XlMqjikI9A
// Timer node wait time = 0.25s (can be modified but this seems good).
// If shorter, it may time out before someone lifts their finger and
// won't be registered.

public partial class SwipeDetector : Node
{
	[Signal]
	public delegate void SwipedEventHandler(Vector2 direction);

	[Signal]
	public delegate void SwipedCancelledEventHandler(Vector2 startPosition);

	// Used to detect diagonal swipes.
	// Adjust to make more or less tolerant to diagonal swiping.
	[Export]
	public float MaxDiagonalSlope { get; set; } = 1.3f;

	private Timer _timer;
	private Vector2 _swipeStartPosition;

	public override void _Ready()
	{
		_timer = GetNode<Timer>("Timer");
		_timer.Timeout += OnTimerTimeout;
	}

	public override void _Input(InputEvent @event)
	{
		//GD.Print("Input received: ", @event);
		//if (@event is not InputEventScreenTouch touchEvent)
		//{
			//GD.Print("Not a touch event, was: ", @event.GetType());
			//return;
		//} else {
			//GD.Print("Touch event detected, pressed: ", touchEvent.Pressed);
		//}

		if (@event is not InputEventScreenTouch touchEvent)
			return;

		if (touchEvent.Pressed)
			StartDetection(touchEvent.Position);
		else if (!_timer.IsStopped())
			EndDetection(touchEvent.Position);
	}

	private void StartDetection(Vector2 position)
	{
		_swipeStartPosition = position;
		_timer.Start();
	}

	private void EndDetection(Vector2 position)
	{
		_timer.Stop();

		// Calculate the direction of the swipe.
		// Normalizing returns a Vector2 with values of 0 - 1 on both axes.
		Vector2 direction = (position - _swipeStartPosition).Normalized();

		if (Mathf.Abs(direction.X) + Mathf.Abs(direction.Y) > MaxDiagonalSlope)
			// Swipe was invalid diagonally.
			return;

		if (Mathf.Abs(direction.X) > Mathf.Abs(direction.Y))
			// Horizontal swipe detected.
			// "-Sign" because when a swipe to the right is detected,
			// we actually want the camera to move to the left. Inverts
			// the direction X to send the signal in the correct direction.
			EmitSignal(SignalName.Swiped, new Vector2(-Mathf.Sign(direction.X), 0.0f));
		else
			// Vertical swipe detected.
			EmitSignal(SignalName.Swiped, new Vector2(0.0f, -Mathf.Sign(direction.Y)));
	}

	private void OnTimerTimeout()
	{
		EmitSignal(SignalName.SwipedCancelled, _swipeStartPosition);
	}
}
