// Energy Transitions
// Nat Sci Hall - OMSI
// Calico Rose
// Purpose: Goes through a slideshow on touchscreen monitor.
// If "r" is pressed (simulated by Teensy), reset screen shows
// until "g" is pressed (simulated by Teensy) to enable the slideshow.
// If "s" is pressed, freeze slideshow on whatever slide is up
// and wait for another command (r or g).
// References:
// https://docs.godotengine.org/en/stable/getting_started/step_by_step/signals.html#doc-signals


using Godot;
using System;

public partial class Slideshow : Node2D {
	private bool gameActive       = true;
	private bool eng              = true;
	private bool _spaceWasPressed = false;
	private bool _gWasPressed     = false;
	private bool _rWasPressed     = false;
	private bool _sWasPressed     = false;

	private Vector2 viewportSize;
	private int screenWidth;
	private int screenHeight;
	private int slideNumber;

	[Export] private TextureRect _swipeScreen;
	[Export] private TextureRect _solarScreen;
	[Export] private TextureRect _windScreen;
	[Export] private TextureRect _waveScreen;
	[Export] private TextureRect _hydroScreen;
	[Export] private TextureRect _geoScreen;
	[Export] private TextureRect _resetScreen;

	[Export] private Texture2D swipeEng;
	[Export] private Texture2D swipeSpanish;
	[Export] private Texture2D solarEng;
	[Export] private Texture2D solarSpanish;
	[Export] private Texture2D windEng;
	[Export] private Texture2D windSpanish;
	[Export] private Texture2D waveEng;
	[Export] private Texture2D waveSpanish;
	[Export] private Texture2D hydroEng;
	[Export] private Texture2D hydroSpanish;
	[Export] private Texture2D geoEng;
	[Export] private Texture2D geoSpanish;
	[Export] private Texture2D resetEng;
	[Export] private Texture2D resetSpanish;

	public override void _Ready() {
		// Connect the SwipeDetector node & set up signal listener.
		var swipeNode = GetNode<SwipeDetector>("SwipeDetector");
		swipeNode.Swiped += OnSignalReceived;

		// Set to Fullscreen Mode
		DisplayServer.WindowSetMode(DisplayServer.WindowMode.Fullscreen);
		// Move to the second monitor (index 1)
		//DisplayServer.WindowSetCurrentScreen(1);
		Vector2I screenSize = DisplayServer.ScreenGetSize();
		GD.Print("Screen Resolution: " + screenSize);


		_swipeScreen = GetNode<TextureRect>("CenterContainer/SwipeScreen");
		_solarScreen = GetNode<TextureRect>("CenterContainer/SolarScreen");
		_windScreen  = GetNode<TextureRect>("CenterContainer/WindScreen");
		_waveScreen  = GetNode<TextureRect>("CenterContainer/WaveScreen");
		_hydroScreen = GetNode<TextureRect>("CenterContainer/HydroelectricScreen");
		_geoScreen   = GetNode<TextureRect>("CenterContainer/GeothermalScreen");
		_resetScreen = GetNode<TextureRect>("CenterContainer/ResetScreen");

		resetEng = (Texture2D)GD.Load("res://Media/English/English1.jpg");
		swipeEng = (Texture2D)GD.Load("res://Media/English/English2.jpg");
		solarEng = (Texture2D)GD.Load("res://Media/English/English3.jpg");
		windEng  = (Texture2D)GD.Load("res://Media/English/English4.jpg");
		waveEng  = (Texture2D)GD.Load("res://Media/English/English5.jpg");
		hydroEng = (Texture2D)GD.Load("res://Media/English/English6.jpg");
		geoEng   = (Texture2D)GD.Load("res://Media/English/English7.jpg");

		resetSpanish = (Texture2D)GD.Load("res://Media/Spanish/Spanish1.jpg");
		swipeSpanish = (Texture2D)GD.Load("res://Media/Spanish/Spanish2.jpg");
		solarSpanish = (Texture2D)GD.Load("res://Media/Spanish/Spanish3.jpg");
		windSpanish  = (Texture2D)GD.Load("res://Media/Spanish/Spanish4.jpg");
		waveSpanish  = (Texture2D)GD.Load("res://Media/Spanish/Spanish5.jpg");
		hydroSpanish = (Texture2D)GD.Load("res://Media/Spanish/Spanish6.jpg");
		geoSpanish   = (Texture2D)GD.Load("res://Media/Spanish/Spanish7.jpg");

		// Get viewport size as a Vector2.
		viewportSize = GetViewport().GetVisibleRect().Size;

		screenWidth  = (int)viewportSize.X;
		screenHeight = (int)viewportSize.Y;

		SetTexturesToEnglish();

		_resetScreen.Set("visible", false);
		_solarScreen.Set("visible", false);
		_windScreen.Set("visible",  false);
		_waveScreen.Set("visible",  false);
		_hydroScreen.Set("visible", false);
		_geoScreen.Set("visible",   false);
		_swipeScreen.Set("visible", true);

		slideNumber = 0;
	}

	public override void _Process(double delta) {
		bool spaceDown = Input.IsKeyPressed(Key.Space);
		bool rPressed  = Input.IsKeyPressed(Key.R);
		bool sPressed  = Input.IsKeyPressed(Key.S);
		bool gPressed  = Input.IsKeyPressed(Key.G);

		// Make sure it only fire once per key press, not every frame
		if (spaceDown && !_spaceWasPressed) {
			OnSpacePressed();
		}
		_spaceWasPressed = spaceDown;

		// Needs to be reset.
		if (rPressed && !_rWasPressed) {
			OnRPressed();
		}
		_rWasPressed = rPressed;

		// Results are showing.
		if(sPressed && !_sWasPressed) {
			OnSPressed();
		}
		_sWasPressed = sPressed;

		// Activate game.
		if(gPressed && !_gWasPressed) {
			OnGPressed();
		}
		_gWasPressed = gPressed;
	}

	private void OnSignalReceived(Vector2 direction) {
		if(!gameActive) {
			return;
		}

		if(direction.X == 1) {
			// Swiping to the left (wanting the slideshow to move to the right) = (1, 0)
			// slideNumber 0 = _swipeScreen
			// slideNumber 1 = _solarScreen
			// slideNumber 2 = _windScreen
			// slideNumber 3 = _waveScreen
			// slideNumber 4 = _hydroScreen
			// slideNumber 5 = _geoScreen
			switch(slideNumber) {
				case 0:
					// Move from swipe screen to solar screen
					SetScreenVisibility(false, true, false, false, false, false, false);
					slideNumber++;
					break;
				case 1:
					// Move from solar screen to wind screen
					SetScreenVisibility(false, false, true, false, false, false, false);
					slideNumber++;
					break;
				case 2:
					// Move from wind screen to wave screen
					SetScreenVisibility(false, false, false, true, false, false, false);
					slideNumber++;
					break;
				case 3:
					// Move from wave screen to hydro screen
					SetScreenVisibility(false, false, false, false, true, false, false);
					slideNumber++;
					break;
				case 4:
					// Move from hydro screen to geo screen
					SetScreenVisibility(false, false, false, false, false, true, false);
					slideNumber++;
					break;
				case 5:
					// On last page, geo page, don't do anything.
					break;
				default:
					break;
			}
		} else if (direction.X == -1) {
			// Swiping to the right (wanting the slideshow to move to the left) = (-1, 0)
			switch(slideNumber) {
				case 0:
					// On first page, don't do anything
					break;
				case 1:
					// Move from solar screen to swipe screen
					SetScreenVisibility(true, false, false, false, false, false, false);
					slideNumber--;
					break;
				case 2:
					// Move from wind screen to solar screen
					SetScreenVisibility(false, true, false, false, false, false, false);
					slideNumber--;
					break;
				case 3:
					// Move from wave screen to wind screen
					SetScreenVisibility(false, false, true, false, false, false, false);
					slideNumber--;
					break;
				case 4:
					// Move from hydro screen to wave screen
					SetScreenVisibility(false, false, false, true, false, false, false);
					slideNumber--;
					break;
				case 5:
					// Move from geo screen to hydro screen
					SetScreenVisibility(false, false, false, false, true, false, false);
					slideNumber--;
					break;
				default:
					break;
			}
		}
		return;
	}

	private void OnSpacePressed() {
		if(eng) {
			// Change to spanish
			eng = false;
			SetTexturesToSpanish();
		} else if(!eng) {
			// Change to english
			eng = true;
			SetTexturesToEnglish();
		}
	}

	// Activate game.
	private void OnGPressed() {
		gameActive = true;
		SetScreenVisibility(true, false, false, false, false, false, false);
		return;
	}

	// Game needs to be reset.
	private void OnRPressed() {
		gameActive = false;
		SetScreenVisibility(false, false, false, false, false, false, true);
		return;
	}

	// Results are showing.
	private void OnSPressed() {
		gameActive = false;
		return;
	}

	private void SetTexturesToSpanish() {
		if(swipeSpanish == null || solarSpanish == null || windSpanish == null ||
			waveSpanish == null || hydroSpanish == null || geoSpanish == null ||
			resetSpanish == null) {
			return;
		} else {
			_swipeScreen.Texture = swipeSpanish;
			_solarScreen.Texture = solarSpanish;
			_windScreen.Texture  = windSpanish;
			_waveScreen.Texture  = waveSpanish;
			_hydroScreen.Texture = hydroSpanish;
			_geoScreen.Texture   = geoSpanish;
			_resetScreen.Texture = resetSpanish;

			if (resetSpanish == null) GD.PrintErr("Failed to load Spanish1.jpg");
			if (swipeSpanish == null) GD.PrintErr("Failed to load Spanish2.jpg");
			if (solarSpanish == null) GD.PrintErr("Failed to load Spanish3.jpg");
			if (windSpanish  == null) GD.PrintErr("Failed to load Spanish4.jpg");
			if (waveSpanish  == null) GD.PrintErr("Failed to load Spanish5.jpg");
			if (hydroSpanish == null) GD.PrintErr("Failed to load Spanish6.jpg");
			if (geoSpanish   == null) GD.PrintErr("Failed to load Spanish7.jpg");
		}
	}

	private void SetTexturesToEnglish() {
		if(swipeEng == null || solarEng == null || windEng == null || waveEng == null ||
			hydroEng == null || geoEng == null || resetEng == null) {
			return;
		} else {
			_swipeScreen.Texture = swipeEng;
			_solarScreen.Texture = solarEng;
			_windScreen.Texture  = windEng;
			_waveScreen.Texture  = waveEng;
			_hydroScreen.Texture = hydroEng;
			_geoScreen.Texture   = geoEng;
			_resetScreen.Texture = resetEng;

			if (resetEng == null) GD.PrintErr("Failed to load English1.jpg");
			if (swipeEng == null) GD.PrintErr("Failed to load English2.jpg");
			if (solarEng == null) GD.PrintErr("Failed to load English3.jpg");
			if (windEng  == null) GD.PrintErr("Failed to load English4.jpg");
			if (waveEng  == null) GD.PrintErr("Failed to load English5.jpg");
			if (hydroEng == null) GD.PrintErr("Failed to load English6.jpg");
			if (geoEng   == null) GD.PrintErr("Failed to load English7.jpg");
		}
	}

	private void SetScreenVisibility(bool swipe, bool solar, bool wind, bool wave, bool hydro, bool geo, bool reset) {
		_swipeScreen.Visible = swipe;
		_solarScreen.Visible = solar;
		_windScreen.Visible  = wind;
		_waveScreen.Visible  = wave;
		_hydroScreen.Visible = hydro;
		_geoScreen.Visible   = geo;
		_resetScreen.Visible = reset;
		return;
	}
}
