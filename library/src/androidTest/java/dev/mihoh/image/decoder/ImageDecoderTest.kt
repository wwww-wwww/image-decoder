package dev.mihon.image.decoder

import android.content.Context
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.Rect
import android.util.Log
import androidx.test.core.app.ApplicationProvider
import androidx.test.ext.junit.runners.AndroidJUnit4
import org.junit.After
import org.junit.Assert.*
import org.junit.Before
import org.junit.Test
import org.junit.runner.RunWith
import kotlin.math.absoluteValue

@RunWith(AndroidJUnit4::class)
class ImageDecoderTest {

    private lateinit var context: Context
    private val imageFormats = listOf("image.png", "image.heif", "image.avif", "image.jpg", "image.webp", "image.jxl")
    private val decoders = mutableListOf<ImageDecoder?>()

    @Before
    fun setUp() {
        // Initialize context for asset loading
        context = ApplicationProvider.getApplicationContext()
    }

    @After
    fun tearDown() {
        // Ensure all decoders are recycled after tests
        decoders.forEach { it?.recycle() }
    }

    @Test
    fun testImageDecodingForSupportedFormats() {

        // decode a png image using Android lib
        val ref_bitmap = context.assets.open("image.png").use { stream ->
            BitmapFactory.decodeStream(stream)
        }

        for (imageName in imageFormats) {
            val decoder = context.assets.open(imageName).use { stream ->
                return@use ImageDecoder.newInstance(stream)
            }
            assertNotNull("Failed to initialize decoder for $imageName", decoder)

            // Verify decode operation
            val bitmap = decoder?.decode(Rect(0, 0, decoder.width, decoder.height), 1)
            assertNotNull("Bitmap decoding failed for $imageName", bitmap)
            assertTrue("Bitmap width should be greater than 0", bitmap!!.width > 0)
            assertTrue("Bitmap height should be greater than 0", bitmap.height > 0)

            val diff =compareBitmap(ref_bitmap, bitmap)

            Log.i("ImageDecoderTest", "testImageDecodingForSupportedFormats: diff is $diff")

            assertTrue("Bitmap for $imageName, diff too big: $diff", diff < 0.1)

            // Store the decoder for recycling in tearDown
            decoders.add(decoder)
        }
    }

    @Test
    fun testImageDecodingForCropping() {
        // decode a png image using Android lib
        val refBitmap = context.assets.open("image.png").use { stream ->
            BitmapFactory.decodeStream(stream)
        }

        // the image is 600x600
        val crops = listOf(
            Pair(1, Rect(0, 0, 300, 300)),
            Pair(1, Rect(300, 300, 600, 600)),
            Pair(1, Rect(300, 0, 600, 300)),
            Pair(1, Rect(100, 100, 200, 200)),

            Pair(2, Rect(100, 100, 200, 200)),
            // test odd crop rect
            Pair(2, Rect(100, 100, 201, 201)),

            Pair(4, Rect(100, 100, 200, 200)),
            // test non-multiple crop rect
            Pair(4, Rect(100, 100, 202, 202)),
            Pair(4, Rect(100, 100, 203, 203)),
        )


        // specify a crop rect
        for (imageName in imageFormats) {
            Log.i("ImageDecoderTest", "testing $imageName")
            val decoder = context.assets.open(imageName).use { stream ->
                return@use ImageDecoder.newInstance(stream)
            }
            assertNotNull("Failed to initialize decoder for $imageName", decoder)

            for ((sampleSize, cropRect) in crops) {
                Log.i("ImageDecoderTest", "testing $sampleSize $cropRect")
                // crop the reference bitmap
                var refCroppedBitmap = Bitmap.createBitmap(
                    refBitmap,
                    cropRect.left,
                    cropRect.top,
                    cropRect.width(),
                    cropRect.height()
                )

                if (sampleSize > 1) {
                    // downscale the reference bitmap
                    val scaledWidth = refCroppedBitmap.width / sampleSize
                    val scaledHeight = refCroppedBitmap.height / sampleSize
                    val scaledBitmap = Bitmap.createScaledBitmap(refCroppedBitmap, scaledWidth, scaledHeight, true)
                    refCroppedBitmap.recycle()
                    refCroppedBitmap = scaledBitmap
                }

                // Verify decode operation
                val bitmap = decoder?.decode(cropRect, sampleSize)
                assertNotNull("Bitmap decoding failed for $imageName", bitmap)
                assertTrue("Bitmap width should be greater than 0", bitmap!!.width > 0)
                assertTrue("Bitmap height should be greater than 0", bitmap.height > 0)
                assertTrue("Bitmap width (${bitmap.width} should be equal to ref width (${refCroppedBitmap.width}", bitmap.width == refCroppedBitmap.width)
                assertTrue("Bitmap height (${bitmap.height} should be equal to ref height (${refCroppedBitmap.height}", bitmap.height == refCroppedBitmap.height)

                val diff = compareBitmap(refCroppedBitmap, bitmap)

                Log.i("ImageDecoderTest", "testImageDecodingForSupportedFormats: diff is $diff")

                assertTrue("Bitmap for $imageName, diff too big: $diff", diff < 0.2)
            }

            // Store the decoder for recycling in tearDown
            decoders.add(decoder)
        }

    }


    @Test
    fun testImageCropBorders() {
        // decode a png image using Android lib
        var refBitmap = context.assets.open("image.png").use { stream ->
            BitmapFactory.decodeStream(stream)
        }

        // crop the reference bitmap
        refBitmap = Bitmap.createBitmap(refBitmap, 50, 50, 500, 500)

        // the cropped image is 500x500
        val crops = listOf(
            Pair(1, Rect(0, 0, 300, 300)),
            Pair(1, Rect(200, 200, 500, 500)),
            Pair(1, Rect(300, 0, 500, 300)),
            Pair(1, Rect(100, 100, 200, 200)),

            Pair(2, Rect(100, 100, 200, 200)),
            // test odd crop rect
            Pair(2, Rect(100, 100, 201, 201)),

            Pair(4, Rect(100, 100, 200, 200)),
            // test non-multiple crop rect
            Pair(4, Rect(100, 100, 202, 202)),
            Pair(4, Rect(100, 100, 203, 203)),
        )

        // specify a crop rect
        for (imageName in imageFormats) {
            Log.i("ImageDecoderTest", "testing $imageName")
            val decoder = context.assets.open(imageName).use { stream ->
                return@use ImageDecoder.newInstance(stream, true)
            }
            assertNotNull("Failed to initialize decoder for $imageName", decoder)

            for ((sampleSize, cropRect) in crops) {
                Log.i("ImageDecoderTest", "testing $sampleSize $cropRect")
                // crop the reference bitmap
                var refCroppedBitmap = Bitmap.createBitmap(
                    refBitmap,
                    cropRect.left,
                    cropRect.top,
                    cropRect.width(),
                    cropRect.height()
                )

                if (sampleSize > 1) {
                    // downscale the reference bitmap
                    val scaledWidth = refCroppedBitmap.width / sampleSize
                    val scaledHeight = refCroppedBitmap.height / sampleSize
                    val scaledBitmap = Bitmap.createScaledBitmap(refCroppedBitmap, scaledWidth, scaledHeight, true)
                    refCroppedBitmap.recycle()
                    refCroppedBitmap = scaledBitmap
                }

                // Verify decode operation
                val bitmap = decoder?.decode(cropRect, sampleSize)
                assertNotNull("Bitmap decoding failed for $imageName", bitmap)
                assertTrue("Bitmap width should be greater than 0", bitmap!!.width > 0)
                assertTrue("Bitmap height should be greater than 0", bitmap.height > 0)
                assertTrue("Bitmap width (${bitmap.width} should be equal to ref width (${refCroppedBitmap.width}", bitmap.width == refCroppedBitmap.width)
                assertTrue("Bitmap height (${bitmap.height} should be equal to ref height (${refCroppedBitmap.height}", bitmap.height == refCroppedBitmap.height)

                val diff = compareBitmap(refCroppedBitmap, bitmap)

                Log.i("ImageDecoderTest", "testImageDecodingForSupportedFormats: diff is $diff")

                assertTrue("Bitmap for $imageName, diff too big: $diff", diff < 0.2)
            }

            // Store the decoder for recycling in tearDown
            decoders.add(decoder)
        }

    }

    private fun compareBitmap(a: Bitmap, b: Bitmap): Double {
        if (a.width != b.width || a.height != b.height) {
            Log.e("ImageDecoderTest", "compareBitmap: image size mismatch: ${a.width}x${a.height} != ${b.width}x${b.height}")
            return 1.0;
        }

        var accum = 0.0;
        for (y in 0 until a.height) {
            for (x in 0 until a.width) {
                val c1 = a.getColor(x, y)
                val c2 = b.getColor(x, y)
                val dr = c1.red() - c2.red()
                val dg = c1.green() - c2.green()
                val db = c1.blue() - c2.blue()
                accum += dr.absoluteValue + dg.absoluteValue + db.absoluteValue
            }
        }

        accum /= a.width * a.height * 3.0;

        return accum
    }
}
